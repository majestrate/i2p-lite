#include <i2pd/ntcp.h>
#include <i2pd/elg.h>
#include <i2pd/memory.h>
#include <openssl/sha.h>
#include "transport_internal.h"
#include "router_internal.h"
#include "ntcp_conn_hashmap.h"

#include <assert.h>
#include <uv.h>

#define ntcp_router_context(s) ((struct router_context *)(s->serv.data))
#define ntcp_serv_uv_loop(s) ntcp_router_context(s)->loop
#define ntcp_conn_uv_loop(c) ntcp_serv_uv_loop(c->server)

struct ntcp_server
{
  uv_tcp_t serv;
  struct ntcp_conn_hashmap * conns;
  int ipv4; // set to 1 if ipv4 capable otherwise 0
  int ipv6; // set to 1 if ipv6 capable otherwise 0
  int prefer_v6; // set to 1 if we prefer ipv6 over ipv4 otherwise 0
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
  // underlying connection
  struct ntcp_conn * conn;
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
  /** set to 1 if established otherwise 0 */
  int established;
  /** connection handle, user data points to this ntcp_conn */
  uv_tcp_t conn;
  /** ntcp server this connection belongs to */
  struct ntcp_server * server;
  /** read buffer */
  uint8_t readbuff[NTCP_BUFF_SIZE];
  /** write buffer */
  uint8_t writebuff[NTCP_BUFF_SIZE];

  /** ipv4 address in use for remote */
  struct sockaddr_in * addr4;
  /** ipv6 address in use for remote */
  struct sockaddr_in6 * addr6;

  /** router info in use for remote */
  struct router_info * remote;
  /** cached router info ident hash */
  ident_hash remote_ih;

  /** dh exchanger */
  struct elg_DH * ntcp_dh;
  
  /** secret x */
  elg_key privx;
  /** public x */
  elg_key pubx;
  /** secret y */
  elg_key privy;
  /** public y */
  elg_key puby;
  /** derived session key */
  dh_shared_key k;
};
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
  // remove if result is NULL because it's an error
  if (!result) ntcp_conn_hashmap_remove(serv->conns, conn->remote_ih);
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

/** read allocator callback for libuv */
static void ntcp_conn_alloc_cb(uv_handle_t * handle, size_t suggested, uv_buf_t *buf)
{
  struct ntcp_conn * conn = (struct ntcp_conn *) handle->data;
  buf->base = &conn->readbuff[0];
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

/** write from write buffer to raw tcp connection, does not copy buf */
static void ntcp_conn_raw_write(struct ntcp_conn * conn, uint8_t * buf, size_t sz)
{
  uv_write_t * w = xmalloc(sizeof(uv_write_t));  
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


static void ntcp_conn_gen_session_request(uv_work_t * work)
{
  struct ntcp_conn_connect_event * ev = (struct ntcp_conn_connect_event *) work->data;
  struct ntcp_conn * conn = ev->conn;
  // generate x, X
  elg_keygen(&conn->privx, &conn->pubx);
  memcpy(conn->writebuff, conn->pubx, sizeof(elg_key));
  // H(X)
  uint8_t * b = conn->writebuff + sizeof(elg_key);
  SHA256(b, sizeof(elg_key), conn->pubx);
  uint8_t * ihb = &conn->remote_ih[0];
  // H(X) xor Bob.ident_hash
  for (int idx = 0; idx < (sizeof(ident_hash) / sizeof(uint64_t)); idx ++) {
    uint64_t * ih = (uint64_t *) ihb;
    uint64_t * bh = (uint64_t *) b;
    *bh = (*ih) ^ (*bh);
    ihb += sizeof(uint64_t);
    b += sizeof(uint64_t);
  }
}

static void ntcp_conn_after_gen_session_request(uv_work_t * work, int status)
{
  struct ntcp_conn_connect_event * ev = (struct ntcp_conn_connect_event *) work->data;
  // send actual request
}


/** outbound connection callback from uv */
static void ntcp_conn_outbound_connect_cb(uv_connect_t * req, int status)
{
  struct ntcp_conn_connect_event * ev = (struct ntcp_conn_connect_event *) req->data;
  char buf[1024] = {0}; // for error message
  if(status) {
    // fail
    snprintf(buf, sizeof(buf), "failed to connect to %s:%s, %s", ev->host, ev->port, uv_strerror(status));
    char * msg = strdup(buf);
    // call hooks with fail
    ntcp_conn_call_established_hooks(ev->conn, NULL, msg);
    ntcp_free_connect_event(&ev);
    free(msg);
  } else {
    // successfully connected, generate session request
    uv_work_t * work = xmalloc(sizeof(uv_work_t));
    work->data = ev;
    status = uv_queue_work(ntcp_conn_uv_loop(ev->conn), work, ntcp_conn_gen_session_request, ntcp_conn_after_gen_session_request);
    if(status) {
      // fail to queue work
      snprintf(buf, sizeof(buf), "failed to queue ntcp session generation for %s:%s, %s", ev->host, ev->port, uv_strerror(status));
      char * msg = strdup(buf);
      // call hooks with fail
      ntcp_conn_call_established_hooks(ev->conn, NULL, msg);
      ntcp_free_connect_event(&ev);
      free(msg);
    }
  }
}

/** dns resolver result callback for libuv */
static void ntcp_conn_handle_resolve(uv_getaddrinfo_t * req, int status, struct addrinfo * result)
{
  char buf[1024] = {0}; // for error message
  struct ntcp_conn_connect_event * ev = (struct ntcp_conn_connect_event *) req->data;
  if(status) {
    // resolve error
    snprintf(buf, sizeof(buf), "could not resolve %s:%s, %s", ev->host, ev->port, uv_strerror(status));
    char * msg = strdup(buf);
    ntcp_conn_call_established_hooks(ev->conn, NULL, msg);
    ntcp_free_connect_event(&ev);
    free(msg);
  } else {
    // resolve success, populate connection addresses
    struct addrinfo * a = result;
    while(a) {
      if(a->ai_family == AF_INET && ev->conn->server->ipv4) {
        ev->conn->addr4 = xmalloc(sizeof(struct sockaddr_in));
        memcpy(ev->conn->addr4, a->ai_addr, a->ai_addrlen);
      } else if(a->ai_family == AF_INET6 && ev->conn->server->ipv6) {
        ev->conn->addr6 = xmalloc(sizeof(struct sockaddr_in6));
        memcpy(ev->conn->addr6, a->ai_addr, a->ai_addrlen);
      }
      a = a->ai_next;
    }

    struct sockaddr * saddr = NULL;
    if (ev->conn->server->prefer_v6) {
      if(ev->conn->addr6)
        saddr = (struct sockaddr *) ev->conn->addr6;
      else
        saddr = (struct sockaddr *) ev->conn->addr4;
    } else {
      if(ev->conn->addr4)
        saddr = (struct sockaddr *) ev->conn->addr4;
      else
        saddr = (struct sockaddr *) ev->conn->addr6;
    }
    
    if (saddr) {
      // resolve succses, try connecting
      int r = uv_tcp_connect(&ev->connect, &ev->conn->conn, saddr, ntcp_conn_outbound_connect_cb);
      if(r) {
        // libuv connect fail
        snprintf(buf, sizeof(buf), "uv_connect failed: %s", uv_strerror(r));
        char * msg = strdup(buf);
        ntcp_conn_call_established_hooks(ev->conn, NULL, msg);
        ntcp_free_connect_event(&ev);
        free(msg);
      }
    } else {
      // no routable addresses found
      snprintf(buf, sizeof(buf), "could not find any routable addresses to %s:%s", ev->host, ev->port);
      char * msg = strdup(buf);
      ntcp_conn_call_established_hooks(ev->conn, NULL, msg);
      ntcp_free_connect_event(&ev);
      free(msg);
    }
  }
}   



/** allocate new connect event and fill members */
static void ntcp_new_connect_event(struct ntcp_conn_connect_event **e, struct ntcp_conn * conn, struct i2p_addr * addr)
{
  struct ntcp_conn_connect_event * ev = (struct ntcp_conn_connect_event *) xmalloc(sizeof(struct ntcp_conn_connect_event));
  ev->conn = conn;
  ev->host = strdup(addr->host);
  ev->port = i2p_addr_port_str(addr);
  ev->resolve.data = ev;
  *e = ev;
}

static int ntcp_conn_try_connect(struct ntcp_conn * conn, struct i2p_addr * addr)
{
  struct ntcp_conn_connect_event * ev = NULL;
  ntcp_new_connect_event(&ev, conn, addr);
  return uv_getaddrinfo(ntcp_conn_uv_loop(conn), &ev->resolve, ntcp_conn_handle_resolve, ev->host, ev->port, NULL);
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
}

void ntcp_server_attach(struct ntcp_server * s, struct i2np_transport * t)
{
  assert(uv_tcp_init(t->loop, &s->serv) != -1);
  s->serv.data = t->router;
}

// called when ntcp server closes socket
void ntcp_server_closed_callback(uv_handle_t * h)
{
  struct router_context * router = (struct router_context *) h->data;
  ntcp_server_free(&router->ntcp);
}

void ntcp_server_detach(struct ntcp_server * s)
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
    if (conn->established) 
      hook(s, conn, NULL, u); // if not established then it's already in the process of being obtained
    else
      ntcp_conn_add_established_hook(conn, hook, u); // add hook to be called after established
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
