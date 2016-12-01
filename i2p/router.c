#include "router_internal.h"
#include "transport_internal.h"
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/util.h>

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
  (*ctx)->floodfill = cfg.floodfill;
  (*ctx)->reseed = cfg.reseed_url;

  char * dir = path_join(cfg.datadir, "netDb", 0);
  // init netdb parameters
  i2p_netdb_new(&(*ctx)->netdb, dir);
  free(dir);
  
  // init transports
  i2np_transport_new(&(*ctx)->transport, (*ctx)->loop);

  // inject router context
  (*ctx)->transport->router = *ctx;
  
  // alloc/configure ntcp
  ntcp_server_alloc(&(*ctx)->ntcp);
  ntcp_server_configure((*ctx)->ntcp, cfg.ntcp);

  // attach ntcp to transports
  ntcp_server_attach((*ctx)->ntcp, (*ctx)->transport);
}

void router_context_free(struct router_context ** ctx)
{
  // free transport muxer
  i2np_transport_free(&(*ctx)->transport);

  // free netdb
  i2p_netdb_free(&(*ctx)->netdb);

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
    i2p_debug(LOG_ROUTER, "creating data directory %s", ctx->data_dir);
    if(mkdir(ctx->data_dir, 0700) == -1) {
      i2p_error(LOG_ROUTER, "failed to create %s: %s", ctx->data_dir, strerror(errno));
      return 0;
    }
  }
  if(!check_file(ctx->router_keys)) {
    // no router.keys 
    // generate our router identity keys, this saves router.keys to disk
    if(!router_context_regenerate_identity(ctx, DEFAULT_IDENTITY_SIG_TYPE)) {
      // error, wut ?
      i2p_error(LOG_ROUTER, "Failed to generate new router identity, %s", strerror(errno));
      return 0;
    }
  }

  // load router.keys
  int fd = open(ctx->router_keys, O_RDONLY);
  if(fd == -1) {
    i2p_error(LOG_ROUTER, "failed to read router identity at %s, %s", ctx->router_keys, strerror(errno));
    return 0;
  } else {
    // load existing keys
    i2p_identity_keys_new(&ctx->privkeys);
    if(i2p_identity_keys_read(ctx->privkeys, fd)) {
      struct router_info_config * ri_conf = NULL;
      i2p_debug(LOG_ROUTER, "loaded router identity private keys from %s", ctx->router_keys);
      router_info_config_new(&ri_conf);
      // only enable ntcp 4
      // TODO: make configurable
      ntcp_config_new(&ri_conf->ntcp);

      ri_conf->ntcp->try_ip4 = 1;
      
      if (ctx->external_addr4)
        ri_conf->ntcp->addr = strdup(ctx->external_addr4);
      if(ctx->external_port4)
        ri_conf->ntcp->addr = strdup(ctx->external_port4);

      // saves router.info to disk
      router_context_update_router_info(ctx, ri_conf);
      router_info_config_free(&ri_conf);
    } else {
      // failed to read identity
      i2p_error(LOG_ROUTER, "failed to parse router identity at %s, %s", ctx->router_keys, strerror(errno));
      i2p_identity_keys_free(&ctx->privkeys);
      close(fd);
      return 0;
    }
    close(fd);
  }
  fd = open(ctx->router_info, O_RDONLY);
  if (fd == -1) {
    i2p_error(LOG_ROUTER, "couldn't load router info file %s, %s", ctx->router_info, strerror(errno));
    i2p_identity_keys_free(&ctx->privkeys);
    return 0;
  } else {
    router_info_new(&ctx->our_ri);
    if(!router_info_load(ctx->our_ri, fd)) {
      i2p_error(LOG_ROUTER, "couldn't parse router info file %s, %s", ctx->router_info, strerror(errno));
      close(fd);
      router_info_free(&ctx->our_ri);
      i2p_identity_keys_free(&ctx->privkeys);
      return 0;
    }
    close(fd);
  }
  if (ctx->our_ri) {
    char * i = router_info_base64_ident(ctx->our_ri);
    i2p_info(LOG_ROUTER, "our router is %s", i);
    free(i);
  } else {
    // wtf? router info not loaded
    i2p_error(LOG_ROUTER, "failed to get our router info, was null");
    i2p_identity_keys_free(&ctx->privkeys);
    return 0;
  }
  
  if(i2p_netdb_ensure_skiplist(ctx->netdb)) {
    // TOOD: should we put our router into network database skiplist?
    // run load up netdb
    i2p_netdb_load_all(ctx->netdb);
  } else {
    i2p_error(LOG_ROUTER, "failed to created netdb skiplist directory, %s", strerror(errno));
    i2p_identity_keys_free(&ctx->privkeys);
    router_info_free(&ctx->our_ri);
    return 0;
  }
  return 1;
}

void router_context_close(struct router_context * ctx, router_context_close_hook hook)
{
  ntcp_server_detach(ctx->ntcp);
  ntcp_server_close(ctx->ntcp);
}

static void router_context_init_netdb(struct router_context * ctx)
{
  size_t sz = i2p_netdb_loaded_peer_count(ctx->netdb);
  if(sz < NETDB_MIN_PEERS) {
    // not enough peers
    if(ctx->floodfill) {
      // try from router info file
      i2p_info(LOG_ROUTER, "no peers, trying bootstrap from router info file at %s", ctx->floodfill);
      struct router_info * ri = NULL;
      router_info_new(&ri);
      int fd = open(ctx->floodfill, O_RDONLY);
      if (fd == -1) {
        // bad file
        i2p_error(LOG_ROUTER, "couldn't open router info file, %s", strerror(errno));
      } else {
        // load from file descriptor
        if(router_info_load(ri, fd)) {
          // start bootstrap attempt
          char * ident = router_info_base64_ident(ri);
          if (router_info_verify(ri)) {
            if(router_info_is_floodfill(ri)) {
              router_context_try_bootstrap_from_floodfill(ctx, ri);
            } else {
              // not a floodfill
              i2p_error(LOG_ROUTER, "%s is not a floodfill router", ident);
              router_info_free(&ri);
            }
          } else {
            // invalid file or signature
            i2p_error(LOG_ROUTER, "%s is invalid", ident);
            router_info_free(&ri);
          }
          free(ident);
        } else {
          // invalid format
          i2p_error(LOG_ROUTER, "%s does not look like a router info", ctx->floodfill);
          router_info_free(&ri);
        }
        close(fd);
      }
    } else if (ctx->reseed) {
      // bootstrap from reseed
      router_context_try_reseed_from(ctx, ctx->reseed);
    } else {
      i2p_error(LOG_ROUTER, "no floodfill to bootstrap from and reseed not enabled");
    }
  } else {
    i2p_info(LOG_ROUTER, "we have loaded %ul peers", sz);
  }
  
}

int router_context_regenerate_identity(struct router_context * ctx, uint16_t sigtype)
{
  int res = 0;
  struct i2p_identity_keys * k = NULL;
  int fd = open(ctx->router_keys, O_CREAT | O_WRONLY, 0600);
  if(fd == -1) return 0;
  i2p_identity_keys_new(&k);
  i2p_identity_keys_generate(k, sigtype);
  if(i2p_identity_keys_write(k, fd)) res = 1;
  close(fd);
  return res;
}

void router_context_try_bootstrap_from_floodfill(struct router_context * ctx, struct router_info * ri)
{
  if(router_info_is_floodfill(ri)) {
    char * ident = router_info_base64_ident(ri);
    i2p_info(LOG_ROUTER, "try bootstrap from %s", ident);
    // do floodfill boostrap here
    
    free(ident);
  }
}

void router_context_try_reseed_from(struct router_context * ctx, const char * url)
{
  if(!url) return;
}


void router_context_run(struct router_context * ctx)
{
  
}

void router_context_update_router_info(struct router_context * ctx, struct router_info_config * conf)
{
}

void router_info_config_new(struct router_info_config ** c)
{
  *c = xmalloc(sizeof(struct router_info_config));
}

void router_info_config_free(struct router_info_config ** c)
{
  if((*c)->ntcp) ntcp_config_free(&(*c)->ntcp);
  free((*c)->ssu);
  free((*c)->caps);
  free(*c);
  *c = NULL;
}
