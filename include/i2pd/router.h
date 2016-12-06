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

  /** @brief event loop to use */
  uv_loop_t * loop;
  
  /** @brief root data directory */
  i2p_filename datadir;

  /** @brief path to floodfill router info to try to bootstrap from */
  char * floodfill;

  /** @brief url to reseed server to try to bootstrap from */
  char * reseed_url;
  
  /** @brief config for ntcp */
  struct ntcp_config ntcp;

  /** @brief config for ssu */
  struct ssu_config ssu;
};

#define default_router_context_config { NULL, {0}, NULL, NULL, default_ntcp_config, default_ssu_config }

struct router_context;

/** @brief initialize router context, does not load anything */
void router_context_new(struct router_context ** ctx, struct router_context_config c);
/** @brief free stopped router context */
void router_context_free(struct router_context ** ctx);

/** @brief load i2p router context internal members, return 1 on success otherwise return 0 if any errors happen */
int router_context_load(struct router_context * ctx);

/** @brief regenerate router identity keys, save to disk */
int router_context_regenerate_identity(struct router_context * ctx, uint16_t sigtype);

/** @brief update router info information, sign, save and publish (if publishing is enabled), if conf is NULL this does nothing */
void router_context_update_router_info(struct router_context * ctx, struct router_info_config * conf);

/** @brief try bootstrapping from floodfill */
void router_context_try_bootstrap_from_floodfill(struct router_context * ctx, struct router_info * ri);

void router_context_get_identity(struct router_context * ctx, struct i2p_identity ** ident);

/** @brief start reseed by url, if i2p domain and no peers this does nothing */
void router_context_try_reseed_from(struct router_context * ctx, const char * url);

/** @brief run router context on it's mainloop, issues events and returns */
void router_context_run(struct router_context * ctx);

typedef void (*router_context_close_hook)(struct router_context *, void *);

/** @brief close router context */
void router_context_close(struct router_context * ctx, router_context_close_hook hook);

/** @brief find i2np message router that handles an i2np message given the type of i2np message */
struct i2np_message_router * router_context_find_message_router(struct router_context * ctx, uint8_t type);

#endif
