#ifndef I2PD_NTCP_SERVER_H_
#define I2PD_NTCP_SERVER_H_

#include "router.h"

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

#endif
