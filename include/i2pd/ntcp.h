#ifndef I2PD_NTCP_H_
#define I2PD_NTCP_H_

#include "router.h"
#include "address.h"

/** @brief settings for initializing ntcp server */
struct ntcp_server_settings
{
  /** @brief parent router context */
  struct router_context * router;
};

struct ntcp_server;
/** @brief allocate new ntcp_server */
struct ntcp_server * 
ntcp_server_init(struct ntcp_server_settings settings);

/** @brief bind socket */
int
ntcp_server_bind(struct ntcp_server * s, struct i2p_bind_addr addrs[]);


/** @brief close socket and deallocate */
void
ntcp_server_close(struct ntcp_server * s);


#endif
