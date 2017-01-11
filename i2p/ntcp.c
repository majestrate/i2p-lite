#include <i2pd/ntcp.h>
#include <i2pd/elg.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>
#include <openssl/sha.h>
#include "transport_internal.h"
#include "router_internal.h"
#include "ntcp_conn_hashmap.h"

#include <assert.h>
#include <uv.h>

#define ntcp_router_context(s) (s->transport->router)
#define ntcp_serv_uv_loop(s) ntcp_router_context(s)->loop
#define ntcp_conn_uv_loop(c) ntcp_serv_uv_loop(c->server)


#define NTCP_CONN_CONNECTING 0
#define NTCP_CONN_SESSION_REQUEST 1
#define NTCP_CONN_SESSION_CREATED 2
#define NTCP_CONN_SESSION_CONFIRM_A 3
#define NTCP_CONN_SESSION_CONFIRM_B 4
#define NTCP_CONN_ESTABLISHED 5

#define ntcp_conn_is_connected(c) (c->state != NTCP_CONN_CONNECTING)
#define ntcp_conn_is_established(c) (c->state == NTCP_CONN_ESTABLISHED)

void ntcp_config_new(struct ntcp_config ** c)
{
  *c = xmalloc(sizeof(struct ntcp_config));
}

void ntcp_config_free(struct ntcp_config ** c)
{
  free((*c)->addr);
  free(*c);
  *c = NULL;
}

/** @brief event for handling close of ntcp server socket */
struct ntcp_server_close_hook
{
  ntcp_server_close_handler hook;
  void * user;
  struct ntcp_server_close_hook * next;
};

struct ntcp_server
{
  uv_tcp_t serv;
  struct router_context * router;
  struct ntcp_conn_hashmap * conns;
  int ipv4; // set to 1 if ipv4 capable otherwise 0
  int ipv6; // set to 1 if ipv6 capable otherwise 0
  int prefer_v6; // set to 1 if we prefer ipv6 over ipv4 otherwise 0
  struct i2np_transport_impl i2np_impl; // i2np transport implementation hooks
  struct i2np_transport * transport; // i2np transport parent
  struct ntcp_server_close_hook * close_hooks;
};

// hook to be called after connection is established and handshake done
struct ntcp_conn_established_hook
{
  ntcp_conn_visiter hook;
  void * user;
  /** pointer to next event or NULL if no more hooks to call */
  struct ntcp_conn_established_hook * next;
};

/** connect to outbound router via ntcp event */
struct ntcp_conn_connect_event
{
  // dns resolver handle
  uv_getaddrinfo_t resolve;
  // uv connect handle
  uv_connect_t connect;
  char * host;
  char * port;
};

/** accept inbound ntcp connection event */
struct ntcp_conn_accept_event
{
  struct ntcp_conn * conn;
};

struct ntcp_conn
{
  /** hooks to call on established */
  struct ntcp_conn_established_hook * established_hooks;

  /** current connection attempt */
  struct ntcp_conn_connect_event * connect;
  
  /** establishment state 0 for disconected */
  int state;
  /** connection handle, user data points to this ntcp_conn */
  uv_tcp_t conn;
  /** ntcp server this connection belongs to */
  struct ntcp_server * server;
  /** read buffer */
  uint8_t readbuff[NTCP_BUFF_SIZE];
  /** write buffer */
  uint8_t writebuff[NTCP_BUFF_SIZE];

  /** iaddress in use for remote */
  struct sockaddr * addr;

  /** router info in use for remote */
  struct router_info * remote;
  /** cached router info ident hash */
  ident_hash remote_ih;

  /** dh exchanger */
  struct elg_DH * ntcp_dh;
  
  /** our secret key */
  elg_key priv;
  /** our public key */
  elg_key pub;
  /** derived session key */
  dh_shared_key k;
};

static char * ntcp_conn_get_ip_str(struct ntcp_conn * conn)
{
  char buf[INET6_ADDRSTRLEN] = {0};
  if(conn->addr) {
    if (conn->addr->sa_family == AF_INET) {
      struct sockaddr_in * in4 = (struct sockaddr_in *) conn->addr;
      if(!inet_ntop(AF_INET, &in4->sin_addr, buf, 4)) {
        snprintf(buf, sizeof(buf), "[bad ipv4]");
      }
    } else if(conn->addr->sa_family == AF_INET6) {
      struct sockaddr_in6 * in6 = (struct sockaddr_in6 *) conn->addr;
      if(!inet_ntop(AF_INET6, &in6->sin6_addr.s6_addr, buf, 16)) {
        snprintf(buf, sizeof(buf), "[bad ipv6]");
      }
    } else {
      snprintf(buf, sizeof(buf), "[bad af: %d]", conn->addr->sa_family);
    }
  } else {
    snprintf(buf, sizeof(buf), "[no address]");
  }
  return strndup(buf, sizeof(buf));
}

/** call all hooks and free them after calling each */
static void ntcp_conn_call_established_hooks(struct ntcp_conn * conn, struct ntcp_conn * result, const char * msg)
{
  struct ntcp_server * serv = conn->server;
  struct ntcp_conn_established_hook * cur = conn->established_hooks;
  struct ntcp_conn_established_hook * next = NULL;
  while(cur) {
    // call hook
    if (cur->hook)
      cur->hook(serv, result, msg, cur->user);
    next = cur->next;
    // free hook
    free(cur);
    // next hook
    cur = next;
  }
  conn->established_hooks = NULL;
}

static void ntcp_conn_add_established_hook(struct ntcp_conn * conn, ntcp_conn_visiter visit, void * user)
{
  struct ntcp_conn_established_hook * hook = (struct ntcp_conn_established_hook*) xmalloc(sizeof(struct ntcp_conn_established_hook));
  hook->user = user;
  hook->hook = visit;
  // append
  if (conn->established_hooks)
  {
    struct ntcp_conn_established_hook * base = conn->established_hooks;
    while(base->next) base = base->next; // get end of list
    base->next = hook; 
  }
  else // set as base
    conn->established_hooks = hook;
}

/** free connect event */
static void ntcp_free_connect_event(struct ntcp_conn_connect_event ** e)
{
  struct ntcp_conn_connect_event * ev = *e;
  free(ev->host);
  free(ev->port);
  free(ev);
  *e = NULL;
}

/** read allocator callback for libuv */
static void ntcp_conn_read_alloc_cb(uv_handle_t * handle, size_t suggested, uv_buf_t *buf)
{
  struct ntcp_conn * conn = (struct ntcp_conn *) handle->data;
  buf->base = (char *) conn->readbuff;
  if(suggested > NTCP_BUFF_SIZE) {
    buf->len = NTCP_BUFF_SIZE;
  } else {
    buf->len = suggested;
  }
}

/** handle close callback for libuv */
static void ntcp_conn_close_cb(uv_handle_t * h)
{
  struct ntcp_conn * conn = (struct ntcp_conn*) h->data;
  ntcp_conn_hashmap_remove(conn->server->conns, conn->remote_ih);
  free(conn);
}

/** queued write request */
struct ntcp_write_event
{
  uv_write_t req;
  uv_buf_t buffer;
  struct ntcp_conn * conn;
};

static void ntcp_conn_close(struct ntcp_conn * conn, char * errormsg)
{
  ntcp_conn_call_established_hooks(conn, NULL, errormsg);
  if(conn->connect) {
      ntcp_free_connect_event(&conn->connect);
  }
  uv_close((uv_handle_t*) &conn->conn, ntcp_conn_close_cb);
}

static void ntcp_conn_wrote_buffer(uv_write_t * w, int status)
{
  struct ntcp_write_event * ev = (struct ntcp_write_event *) w->data;
  if(status) {
    char buf[1024] = {0};
    snprintf(buf, sizeof(buf), "ntcp conn write buffer failed: %s", uv_strerror(status));
    ntcp_conn_close(ev->conn, buf);
  }
  free(ev->buffer.base);
  free(ev);
}

/** queue write from write buffer to raw tcp connection, copies buf */
static void ntcp_conn_raw_write(struct ntcp_conn * conn, uint8_t * buf, size_t sz)
{
  struct ntcp_write_event * ev = xmalloc(sizeof(struct ntcp_write_event));
  ev->req.data = ev;
  
  ev->buffer.base= xmalloc(sz);
  ev->buffer.len = sz;
  memcpy(ev->buffer.base, buf, sz);
  
  uv_write(&ev->req, (uv_stream_t*) &conn->conn, &ev->buffer, 1, ntcp_conn_wrote_buffer);
}

/** uv_work handler for generating handshake 2 */
static void ntcp_conn_gen_session_created(uv_work_t * work)
{
  struct ntcp_conn * conn = (struct ntcp_conn *) work->data;
  struct router_context * router = ntcp_router_context(conn->server);
  struct i2p_identity * our_ident = NULL;
  router_info_get_identity(router->our_ri, &our_ident);

  // get our ident hash
  ident_hash our_hash;
  i2p_identity_hash(our_ident, &our_hash);

  // extract X
  memcpy(conn->pub, conn->readbuff, sizeof(elg_key));
  // verify HXxorHI
  ident_hash digest;
  SHA256(conn->pub, sizeof(elg_key), digest);
  if (memcmp(digest, our_hash, sizeof(ident_hash))) {
    // key missmatch
    char * ip = ntcp_conn_get_ip_str(conn);
    i2p_error(LOG_NTCP, "ntcp conn session request key missmatch from %s", ip);
    free(ip);
  }
  
  // generate our keys
  elg_keygen(&conn->priv, &conn->pub);
  
}

/** uv_work handler for generating handshake 1 */
static void ntcp_conn_gen_session_request(uv_work_t * work)
{
  struct ntcp_conn * conn = (struct ntcp_conn *) work->data;
  // generate x, X
  elg_keygen(&conn->priv, &conn->pub);
  memcpy(conn->writebuff, conn->pub, sizeof(elg_key));
  // H(X)
  uint8_t * b = conn->writebuff + sizeof(elg_key);
  SHA256(b, sizeof(elg_key), conn->pub);
  uint8_t * ihb = conn->remote_ih;
  // H(X) xor Bob.ident_hash
  for (int idx = 0; idx < (sizeof(ident_hash) / sizeof(uint64_t)); idx ++) {
    uint64_t * ih = (uint64_t *) ihb;
    uint64_t * bh = (uint64_t *) b;
    *bh = (*ih) ^ (*bh);
    ihb += sizeof(uint64_t);
    b += sizeof(uint64_t);
  }
}

static void ntcp_conn_compute_session_key(uv_work_t * work)
{
  struct ntcp_conn * conn = (struct ntcp_conn *) work->data;
}

static void ntcp_conn_handle_raw_read(uv_stream_t * s, ssize_t nread, const uv_buf_t * buf)
{
  char msg[1024] = {0};
  struct ntcp_conn * conn = (struct ntcp_conn *) s->data;
  if(conn->state == NTCP_CONN_SESSION_REQUEST) {
    // handshake 1 sent, we got handshake 2
    if(nread == 304) {
      // TODO: allow multiple segments
      
    } else {
      snprintf(msg, sizeof(msg), "bad handshake 2 size: %lu", nread);
      ntcp_conn_close(conn, msg);
    }
    
  }
}

static void ntcp_conn_after_gen_session_request(uv_work_t * work, int status)
{
  char buf[1024] = {0};
  struct ntcp_conn * conn = (struct ntcp_conn *) work->data;
  if(status) {
    snprintf(buf, sizeof(buf), "ntcp connection generate session request failed: %s", uv_strerror(status));
    ntcp_conn_close(conn, buf);
  } else {
    // send actual request
    ntcp_conn_raw_write(conn, conn->writebuff, 288);
    // start reading data from remote
    status = uv_read_start((uv_stream_t*)&conn->conn, ntcp_conn_read_alloc_cb, ntcp_conn_handle_raw_read);
    if (status) {
      snprintf(buf, sizeof(buf), "ntcp connection could not start reading: %s", uv_strerror(status));
      ntcp_conn_close(conn, buf);
    }
  }
}


/** outbound connection callback from uv */
static void ntcp_conn_outbound_connect_cb(uv_connect_t * req, int status)
{
  struct ntcp_conn * conn = (struct ntcp_conn *) req->data;
  struct ntcp_conn_connect_event * ev = conn->connect;
  char buf[1024] = {0}; // for error message
  if(status) {
    // fail
    snprintf(buf, sizeof(buf), "failed to connect to %s:%s, %s", ev->host, ev->port, uv_strerror(status));
    char * msg = strdup(buf);
    // call hooks with fail
    ntcp_conn_call_established_hooks(conn, NULL, msg);
    ntcp_free_connect_event(&conn->connect);
    free(msg);
  } else {
    char * ip = ntcp_conn_get_ip_str(conn);
    i2p_debug(LOG_NTCP, "ntcp connected to %s", ip);
    free(ip);
    // successfully connected, generate session request
    uv_work_t * work = xmalloc(sizeof(uv_work_t));
    work->data = conn;
    status = uv_queue_work(ntcp_conn_uv_loop(conn), work, ntcp_conn_gen_session_request, ntcp_conn_after_gen_session_request);
    if(status) {
      // fail to queue work
      snprintf(buf, sizeof(buf), "failed to queue ntcp session generation for %s:%s, %s", conn->connect->host, conn->connect->port, uv_strerror(status));
      char * msg = strdup(buf);
      // call hooks with fail
      ntcp_conn_call_established_hooks(conn, NULL, msg);
      ntcp_free_connect_event(&conn->connect);
      free(msg);
      // close connection because of queue work fail
      uv_close((uv_handle_t*)&conn->conn, ntcp_conn_close_cb);
    } else {
      // we connected fine and queued session request generate work fine
      // free connect event, no longer required
      ntcp_free_connect_event(&conn->connect);
      // set state accordingly
      conn->state = NTCP_CONN_SESSION_REQUEST;
    }
  }
}

/** dns resolver result callback for libuv */
static void ntcp_conn_handle_resolve(uv_getaddrinfo_t * req, int status, struct addrinfo * result)
{
  char buf[1024] = {0}; // for error message
  struct ntcp_conn * conn = (struct ntcp_conn *) req->data;
  if(status) {
    // resolve error
    snprintf(buf, sizeof(buf), "could not resolve %s:%s, %s", conn->connect->host, conn->connect->port, uv_strerror(status));
    char * msg = strdup(buf);
    ntcp_conn_call_established_hooks(conn, NULL, msg);
    ntcp_free_connect_event(&conn->connect);
    free(msg);
  } else {
    // resolve success, populate connection addresses
    // TODO: ipv6
    struct addrinfo * a = result;
    while(a) {
      if(a->ai_family == AF_INET && conn->server->ipv4) {
        struct sockaddr_in * addr = xmalloc(sizeof(struct sockaddr_in));
        memcpy(addr, a->ai_addr, a->ai_addrlen);
        conn->addr = (struct sockaddr *) addr;
        break;
      }
      a = a->ai_next;
    }
    
    if (conn->addr) {
      // resolve succses, initialize tcp handle for connection
      uv_tcp_init(ntcp_conn_uv_loop(conn), &conn->conn);
      // try connecting
      char * ip = ntcp_conn_get_ip_str(conn);
      i2p_debug(LOG_NTCP, "try connecting to %s:%s", ip, conn->connect->port);
      free(ip);
      status = uv_tcp_connect(&conn->connect->connect, &conn->conn, conn->addr, ntcp_conn_outbound_connect_cb);
      if(status) {
        // libuv connect fail
        snprintf(buf, sizeof(buf), "uv_connect failed: %s", uv_strerror(status));
        char * msg = strdup(buf);
        ntcp_conn_call_established_hooks(conn, NULL, msg);
        ntcp_free_connect_event(&conn->connect);
        free(msg);
      } else {
        // TODO: start ntcp connect timeout timer here
      }
    } else {
      // no routable addresses found
      snprintf(buf, sizeof(buf), "could not find any routable addresses to %s:%s", conn->connect->host, conn->connect->port);
      char * msg = strdup(buf);
      ntcp_conn_call_established_hooks(conn, NULL, msg);
      ntcp_free_connect_event(&conn->connect);
      free(msg);
    }
  }
}   



/** allocate new connect event and fill members */
static void ntcp_new_connect_event(struct ntcp_conn_connect_event **e, struct ntcp_conn * conn, struct i2p_addr * addr)
{
  
  struct ntcp_conn_connect_event * ev = (struct ntcp_conn_connect_event *) xmalloc(sizeof(struct ntcp_conn_connect_event));
  ev->host = strdup(addr->host);
  ev->port = i2p_addr_port_str(addr);
  ev->resolve.data = conn;
  *e = ev;
}

static int ntcp_conn_try_connect(struct ntcp_conn * conn, struct i2p_addr * addr)
{
  if(ntcp_conn_is_connected(conn)) return 0; // we are already connected
  if(conn->connect) {
    // get rid of any existing connect event
    ntcp_free_connect_event(&conn->connect);
  }
  ntcp_new_connect_event(&conn->connect, conn, addr);
  // resolve address
  return uv_getaddrinfo(ntcp_conn_uv_loop(conn), &conn->connect->resolve, ntcp_conn_handle_resolve, conn->connect->host, conn->connect->port, NULL);
}


void ntcp_server_alloc(struct ntcp_server ** s)
{
  *s = xmalloc(sizeof(struct ntcp_server));
  ntcp_conn_hashmap_init(&(*s)->conns);
}

void ntcp_server_free(struct ntcp_server ** s)
{
  ntcp_conn_hashmap_free(&(*s)->conns);
  free(*s);
  *s = NULL;
}

void ntcp_server_configure(struct ntcp_server * s, struct ntcp_config c)
{
  // TODO: add settings here, i.e. binding to network interface etc
}

void ntcp_server_attach(struct ntcp_server * s, struct i2np_transport * t)
{
  assert(uv_tcp_init(t->loop, &s->serv) != -1);
  s->serv.data = s;
  s->transport = t;
  i2np_transport_register(t, &s->i2np_impl);
}

void ntcp_server_detach(struct ntcp_server * s)
{
  i2np_transport_deregister(s->transport, &s->i2np_impl);
}

// called when ntcp server closes socket
void ntcp_server_closed_callback(uv_handle_t * h)
{
  struct ntcp_server * serv = (struct ntcp_server *) h->data;
  // call hooks
  struct ntcp_server_close_hook * hook = serv->close_hooks;
  struct ntcp_server_close_hook * cur = hook;
  while(cur) {
    // call the hook if it's set
    if(cur->hook)
      cur->hook(serv, cur->user);

    hook = cur->next;
    free(cur);
    cur = hook;
  }
  // remove from router context
  ntcp_server_free(&ntcp_router_context(serv)->ntcp);
}

void ntcp_server_close(struct ntcp_server * s)
{
  uv_close((uv_handle_t*)&s->serv, ntcp_server_closed_callback);
}

struct ntcp_address_iter_context
{
  int error;
  struct ntcp_conn * conn;
  ntcp_conn_visiter hook;
  void * user;
};

/** called by router_info_iter_addrs to find ntcp addresses, tries connecting to first */
static void ntcp_iter_router_info_addrs(struct router_info * ri, struct i2p_addr * addr, void * user)
{
  struct ntcp_address_iter_context * ctx = (struct ntcp_address_iter_context *) user;
  if(ctx->error) return; // error or success happened
  if (strcmp(addr->style, "NTCP")) return; // not ntcp
  // add hook
  ntcp_conn_add_established_hook(ctx->conn, ctx->hook, ctx->user);
  ctx->error = ntcp_conn_try_connect(ctx->conn, addr);
  
  if(ctx->error == 0) ctx->error = 1; // success
}

void ntcp_server_obtain_conn_by_ident(struct ntcp_server * s, ident_hash h, ntcp_conn_visiter hook, void * u)
{
  struct ntcp_conn * conn = NULL;
  if(ntcp_conn_hashmap_get(s->conns, h, &conn)) {
    // connection exists
    if (ntcp_conn_is_established(conn)) 
      hook(s, conn, NULL, u); // we are already established
    else
      ntcp_conn_add_established_hook(conn, hook, u); // add hook to be called after established, we are still trying to connect
  } else {
    // connection not open
    struct router_info * remote = NULL;
    if(i2p_netdb_find_router_info(ntcp_router_context(s)->netdb, h, &remote)) {
      // found router info
      conn = mallocx(sizeof(struct ntcp_conn), MALLOCX_ZERO);
      conn->server = s;
      memcpy(conn->remote_ih, h, sizeof(ident_hash));
      conn->remote = remote;
      // insert connection into hashmap
      ntcp_conn_hashmap_insert(s->conns, conn);

      // iterate router info addressses and connect to the first (aka "best") one
      struct ntcp_address_iter_context ctx = {0, conn, hook, u};
      router_info_iter_addrs(remote, ntcp_iter_router_info_addrs, &ctx);
      
      if (ctx.error < 0) ntcp_conn_call_established_hooks(conn, NULL, uv_strerror(ctx.error)); // uv error occured
      else if (ctx.error == 0) ntcp_conn_call_established_hooks(conn, NULL, "no ntcp addresses"); // no addresses found
    } else {
      // router not found in netdb
      // TODO: router info lookup
      hook(s, NULL, "router info not found", u);
    }
  }
}

void ntcp_conn_get_ident_hash(struct ntcp_conn * c, ident_hash * h)
{
  memcpy(*h, c->remote_ih, sizeof(ident_hash));
}

void ntcp_config_to_address(struct ntcp_config * c, struct i2p_addr ** a)
{
  if(c && c->addr) {
    *a = xmalloc(sizeof(struct i2p_addr));
    (*a)->style = strdup("NTCP");
    (*a)->host = strdup(c->addr);
    (*a)->port = c->port;
  }
}
