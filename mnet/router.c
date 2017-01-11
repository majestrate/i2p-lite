#include "router_internal.h"
#include "transport_internal.h"
#include <mnet/log.h>
#include <mnet/memory.h>
#include <mnet/util.h>

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>


void router_context_new(struct router_context ** ctx, struct router_context_config cfg)
{
  *ctx = mallocx(sizeof(struct router_context), MALLOCX_ZERO);
  (*ctx)->data_dir = strdup(cfg.datadir);
  (*ctx)->router_info = path_join(cfg.datadir, "router.info", 0);
  (*ctx)->router_keys = path_join(cfg.datadir, "router.keys", 0);
  (*ctx)->loop = cfg.loop;
  (*ctx)->bootstrap = cfg.bootstrap;

  char * dir = path_join(cfg.datadir, "nodes", 0);
  // init netdb parameters
  mnet_netdb_new(&(*ctx)->netdb, dir);
  free(dir);
  
  // init transports
  mnet_garlic_transport_new(&(*ctx)->transport, (*ctx)->loop);

  // inject router context
  (*ctx)->transport->router = *ctx;
  
  // TODO: alloc/configure iwp

  // TODO: attach iwp to transports

  // initialize ticker
  uv_timer_init((*ctx)->loop, &(*ctx)->ticker);
  (*ctx)->ticker.data = *ctx;
}

void router_context_free(struct router_context ** ctx)
{
  // free transport muxer
  mnet_garlic_transport_free(&(*ctx)->transport);

  // free netdb
  mnet_netdb_free(&(*ctx)->netdb);

  // free strings
  free((*ctx)->data_dir);
  free((*ctx)->router_info);
  free((*ctx)->router_keys);
  free((*ctx)->external_addr4);
  free((*ctx)->external_port4);
  free((*ctx)->external_addr6);
  free((*ctx)->external_port6);
  
  // free router context
  free(*ctx);
  *ctx = NULL;
}

int router_context_load(struct router_context * ctx)
{

  if(!check_file(ctx->data_dir)) {
    mnet_debug(LOG_ROUTER, "creating data directory %s", ctx->data_dir);
    if(mkdir(ctx->data_dir, 0700) == -1) {
      mnet_error(LOG_ROUTER, "failed to create %s: %s", ctx->data_dir, strerror(errno));
      return 0;
    }
  }
  if(!check_file(ctx->router_keys)) {
    // no router.keys
    mnet_info(LOG_ROUTER, "%s not found, regenerating router keys", ctx->router_keys);
    // generate our router identity keys, this saves router.keys to disk
    if(!router_context_regenerate_identity(ctx, DEFAULT_IDENTITY_SIG_TYPE)) {
      mnet_error(LOG_ROUTER, "Bad router keys format, regenerating %s", ctx->router_keys);
      // bad format
      del_file(ctx->router_keys);
      // try again
      if(!router_context_regenerate_identity(ctx, DEFAULT_IDENTITY_SIG_TYPE)) {
        // wtf again?
        mnet_error(LOG_ROUTER, "wtf could not regenerate router keys: %s", strerror(errno));
        return 0;
      }
    }
  }

  // load router.keys
  FILE * f = fopen(ctx->router_keys, "r");
  if(!f) {
    mnet_error(LOG_ROUTER, "failed to read router identity at %s, %s", ctx->router_keys, strerror(errno));
    return 0;
  } else {
    // load existing keys
    mnet_identity_keys_new(&ctx->privkeys);
    if(mnet_identity_keys_read(ctx->privkeys, f)) {
      struct router_info_config * ri_conf = NULL;
      mnet_debug(LOG_ROUTER, "loaded router identity private keys from %s", ctx->router_keys);
      router_info_config_new(&ri_conf);
      // TODO: make router info

      // saves router.info to disk
      router_context_update_router_info(ctx, ri_conf);
      router_info_config_free(&ri_conf);
    } else {
      // failed to read identity
      mnet_error(LOG_ROUTER, "failed to parse router identity at %s, %s", ctx->router_keys, strerror(errno));
      mnet_identity_keys_free(&ctx->privkeys);
      fclose(f);
      return 0;
    }
    fclose(f);
  }
  f = fopen(ctx->router_info, "r");
  if (!f) {
    mnet_error(LOG_ROUTER, "couldn't load router info file %s, %s", ctx->router_info, strerror(errno));
    mnet_identity_keys_free(&ctx->privkeys);
    return 0;
  } else {
    router_info_new(&ctx->our_ri);
    if(!router_info_load(ctx->our_ri, f)) {
      mnet_error(LOG_ROUTER, "couldn't parse router info file %s, %s", ctx->router_info, strerror(errno));
      fclose(f);
      router_info_free(&ctx->our_ri);
      mnet_identity_keys_free(&ctx->privkeys);
      return 0;
    }
    fclose(f);
  }
  if (ctx->our_ri) {
    char * i = router_info_base64_ident(ctx->our_ri);
    mnet_info(LOG_ROUTER, "our router is %s", i);
    free(i);
  } else {
    // wtf? router info not loaded
    mnet_error(LOG_ROUTER, "failed to get our router info, was null");
    mnet_identity_keys_free(&ctx->privkeys);
    return 0;
  }
  
  if(mnet_netdb_ensure_skiplist(ctx->netdb)) {
    // TOOD: should we put our router into network database skiplist?
    // run load up netdb
    mnet_netdb_load_all(ctx->netdb);
  } else {
    mnet_error(LOG_ROUTER, "failed to created netdb skiplist directory, %s", strerror(errno));
    mnet_identity_keys_free(&ctx->privkeys);
    router_info_free(&ctx->our_ri);
    return 0;
  }
  return 1;
}

void router_context_close(struct router_context * ctx, router_context_close_hook hook)
{
  // TODO: close iwp

  // stop event ticker
  uv_timer_stop(&ctx->ticker);
}

static void router_context_ensure_min_peers(struct router_context * ctx)
{
  size_t sz = mnet_netdb_loaded_peer_count(ctx->netdb);
  if(sz < NETDB_MIN_PEERS) {
    // not enough peers
    if(ctx->bootstrap) {
      const char * bootstrap = ctx->bootstrap;
      // try from router info file
      mnet_info(LOG_ROUTER, "no peers, trying bootstrap from router info file at %s", bootstrap);
      struct router_info * ri = NULL;
      router_info_new(&ri);
      FILE * f = fopen(bootstrap, "r");
      if (f == NULL) {
        // bad file
        mnet_error(LOG_ROUTER, "couldn't open router info file, %s", strerror(errno));
      } else {
        // load from file descriptor
        if(router_info_load(ri, f)) {
          // start bootstrap attempt
          char * ident = router_info_base64_ident(ri);
          if (router_info_verify(ri)) {
            router_context_try_bootstrap_from_router(ctx, ri);
          } else {
            // invalid file or signature
            mnet_error(LOG_ROUTER, "%s is invalid", ident);
            router_info_free(&ri);
          }
          free(ident);
        } else {
          // invalid format
          mnet_error(LOG_ROUTER, "%s does not look like a router info", bootstrap);
          router_info_free(&ri);
        }
        fclose(f);
      }
    } else {
      mnet_error(LOG_ROUTER, "no node to bootstrap from");
    }
  } else {
    mnet_info(LOG_ROUTER, "we have loaded %ul peers", sz);
  }
  
}

int router_context_regenerate_identity(struct router_context * ctx, uint16_t sigtype)
{
  int res = 0;
  struct mnet_identity_keys * k = NULL;
  FILE * f = fopen(ctx->router_keys, "w");
  if(!f) return 0;
  mnet_identity_keys_new(&k);
  mnet_identity_keys_generate(k, sigtype);
  if(mnet_identity_keys_write(k, f)) res = 1;
  fclose(f);
  return res;
}

void router_context_try_bootstrap_from_router(struct router_context * ctx, struct router_info * ri)
{
  char * ident = router_info_base64_ident(ri);
  mnet_info(LOG_ROUTER, "try bootstrap from %s", ident);
  // do floodfill boostrap here
  free(ident);
}

void router_context_try_reseed_from(struct router_context * ctx, const char * url)
{
  if(!url) return;
  mnet_info(LOG_ROUTER, "try reseeding from %s", url);
}

static void router_context_handle_tick(uv_timer_t * timer)
{
  struct router_context * ctx = (struct router_context * ) timer->data;
  // make sure we have enough peers
  router_context_ensure_min_peers(ctx);
}


void router_context_run(struct router_context * ctx)
{
  // set up tunnel context
  mnet_garlic_tunnel_context_new(ctx, &ctx->tunnels);
  
  // set up exploritory tunnel pool
  mnet_garlic_tunnel_pool_new(ctx->tunnels, &ctx->exploritory_pool);

  // start exploritory pool
  mnet_garlic_tunnel_pool_start(ctx->exploritory_pool);

  // start event ticker
  uv_timer_start(&ctx->ticker, router_context_handle_tick, 0, ROUTER_CONTEXT_TICK_INTERVAL);

}

void router_context_update_router_info(struct router_context * ctx, struct router_info_config * conf)
{
  if(ctx->our_ri) router_info_free(&ctx->our_ri); // free any existing router info

  router_info_generate(ctx->privkeys, conf, &ctx->our_ri); // generate router info and sign
  
  FILE * f = fopen(ctx->router_info, "w+");
  if(f == NULL) {
    // could not open file
    mnet_error(LOG_ROUTER, "Failed to open %s, %s", ctx->router_info, strerror(errno));
    return;
  }
  // write router info
  router_info_write(ctx->our_ri, f);
  fclose(f);
}
