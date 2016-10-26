#ifndef I2PD_NTCP_H_
#define I2PD_NTCP_H_
#include <i2pd/types.h>

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

// forward declare
struct router_context;

struct ntcp_server;

/** @brief allocate new ntcp_server */
void ntcp_server_alloc(struct ntcp_server ** s);

/** @brief configure ntcp server with settings */
void ntcp_server_configure(struct ntcp_server * s, struct ntcp_config c);

/** @brief reconfigure ntcp server while running */
void ntcp_server_reconfigure(struct ntcp_server * s, struct ntcp_config c);

/** @brief return 1 if current running otherwise return 0 */
int ntcp_server_is_running(struct ntcp_server * s);

/** @brief attach ntcp server to a router context */
void ntcp_server_attach(struct ntcp_server * s, struct router_context * c);

/** @brief detach ntcp server from attached router context */
void ntcp_server_detach(struct ntcp_server * s);

/** @brief close sockets and deallocate */
void ntcp_server_free(struct ntcp_server ** s);


#endif
