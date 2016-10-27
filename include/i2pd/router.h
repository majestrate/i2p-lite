#ifndef I2PD_ROUTER_H__
#define I2PD_ROUTER_H__
#include <i2pd/netdb.h>
#include <i2pd/ntcp.h>
#include <i2pd/ssu.h>
#include <i2pd/types.h>

#include <uv.h>

#define I2P_CONFIG_ROUTER_DIR "i2p.router.dir"

/** @brief parameters for initializing a router context */
struct router_context_config
{
  /** @brief root data directory */
  i2p_filename datadir;

  /** @brief config for ntcp */
  struct ntcp_config ntcp;

  /** @brief config for ssu */
  struct ssu_config ssu;
};

#define default_router_context_config { {0}, default_ntcp_config, default_ssu_config }

struct router_context;

/** @brief initialize router context, does not load anything */
void router_context_new(struct router_context ** ctx, struct router_context_config c);
/** @brief free stopped router context */
void router_context_free(struct router_context ** ctx);

/** @brief load i2p router context internal members, return 1 on success otherwise return 0 if any errors happen */
int router_context_load(struct router_context * ctx);

/** @brief run router context on a mainloop, returns immediately */
void router_context_run(struct router_context * ctx, uv_loop_t * loop);

/** @brief close router context */
void router_context_close(struct router_context * ctx);

#endif
