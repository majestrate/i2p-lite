#ifndef I2PD_NTCP_H_
#define I2PD_NTCP_H_
#include <i2pd/transport.h>
#include <i2pd/types.h>

#ifndef NTCP_BUFF_SIZE
#define NTCP_BUFF_SIZE (1024 * 4)
#endif

struct ntcp_config
{
  // address to bind to
  i2p_hostname addr;
  // port to bind to
  uint16_t port;
  // try binding to v4?
  int try_ip4;
  // try binding to v6?
  int try_ip6;
};

#define default_ntcp_config { "0.0.0.0", 1234, 1, 1 }

// forward declare
struct router_context;

/** ntcp server context */
struct ntcp_server;

/** ntcp connection */
struct ntcp_conn;



/** @brief allocate new ntcp_server */
void ntcp_server_alloc(struct ntcp_server ** s);

/** @brief configure ntcp server with settings */
void ntcp_server_configure(struct ntcp_server * s, struct ntcp_config c);

/** @brief return 1 if current running otherwise return 0 */
int ntcp_server_is_running(struct ntcp_server * s);

/** @brief attach ntcp server to an i2np transport layer */
void ntcp_server_attach(struct ntcp_server * s, struct i2np_transport * t);

/** @brief detach ntcp server from previously attached transport layer */
void ntcp_server_detach(struct ntcp_server * s);

/** @brief close server socket */
void ntcp_server_close(struct ntcp_server * s);

/** @brief deallocate */
void ntcp_server_free(struct ntcp_server ** s);

/** @brief get i2np transport implementation for this ntcp server */
struct i2np_transport_impl * ntcp_server_i2np_impl(struct ntcp_server * s);

/** @breif callback for visiting an ntcp connection with ntcp_server_obtian */
typedef void(*ntcp_conn_obtain_cb)(struct ntcp_server*, struct ntcp_conn *, void *);

/** @brief obtain an ntcp connect by router's ident hash, opens session if not already open */
void ntcp_server_obtain_conn_by_ident(struct ntcp_server * s, ident_hash h, ntcp_conn_obtain_cb cb, void * u);

/** @brief get ident hash of connection's remote ident */
void ntcp_conn_get_ident_hash(struct ntcp_conn * c, ident_hash * h);

#endif
