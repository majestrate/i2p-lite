#ifndef I2PD_ROUTER_H__
#define I2PD_ROUTER_H__
#include <i2pd/ntcp.h>
#include <i2pd/ssu.h>
#include <i2pd/types.h>

/** @brief parameters for initializing a router context */
struct router_context_config
{
  /** @brief path to router.info */
  char router_info[MAX_FILENAME_SIZE];
  /** @brief path to router.keys */
  char router_keys[MAX_FILENAME_SIZE];

  /** @brief config for ntcp */
  struct ntcp_config ntcp;

  /** @brief config for ssu */
  struct ssu_config ssu;

};

struct router_context;

void router_context_init(struct router_context ** ctx, struct router_context_config c);

void router_context_close(struct router_context ** ctx);

#endif
