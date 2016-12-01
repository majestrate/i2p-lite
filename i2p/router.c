#include "router_internal.h"
#include "transport_internal.h"
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/util.h>

#include <string.h>
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
  
  // free router context
  free(*ctx);
  *ctx = NULL;
}

int router_context_load(struct router_context * ctx)
{

  if(!check_file(ctx->data_dir)) {
    i2p_info(LOG_ROUTER, "creating data directory %s", ctx->data_dir);
    if(mkdir(ctx->data_dir, 0700) == -1) {
      i2p_error(LOG_ROUTER, "failed to create %s: %s", ctx->data_dir, strerror(errno));
      return 0;
    }
  }
  struct router_identity * identity = NULL;
  struct router_info * our_ri = NULL;
  if(check_file(ctx->router_keys)) {
    // we have it
    int fd = open(ctx->router_keys, O_RDONLY);
    if(fd == -1) {
      i2p_error(LOG_ROUTER, "failed to read router identity at %s, %s", ctx->router_keys, strerror(errno));
      return 0;
    } else {
      router_identity_new(&identity);
      if(router_identity_read(identity, fd)) {
        i2p_info(LOG_ROUTER, "loaded router identity from %s", ctx->router_keys);
        router_identity_get_router_info(identity, &our_ri);
      } else {
        // failed to read identity
        i2p_error(LOG_ROUTER, "failed to parse router identity at %s, %s", ctx->router_keys, strerror(errno));
        router_identity_free(&identity);
        close(fd);
        return 0;
      }
      close(fd);
    }
  } else {
    int fd = open(ctx->router_keys, O_CREAT | O_WRONLY);
    if(fd == -1) {
      i2p_error(LOG_ROUTER, "failed create router identity at %s: %s", ctx->router_keys, strerror(errno));
      return 0;
    } else {
      router_identity_new(&identity);
      // generate router keys
      i2p_info(LOG_ROUTER, "generating new router identity at %s", ctx->router_keys);
      router_identity_regenerate(identity, DEFAULT_ROUTER_IDENTITY_SIG_TYPE);
      router_identity_write(identity, fd);
      close(fd);
      router_identity_get_router_info(identity, &our_ri);
      // write our router info
      if (check_file(ctx->router_info))
        fd = open(ctx->router_info, O_WRONLY);
      else
        fd = open(ctx->router_info, O_CREAT | O_WRONLY);
      if(fd == -1) {
        i2p_error(LOG_ROUTER, "failed to open router info file for writing, %s", strerror(errno));
        router_identity_free(&identity);
        router_info_free(&our_ri);
        return 0;
      } else {
        router_info_write(our_ri, fd);
        close(fd);
      }
    }
  }
  if (our_ri) {
    char * i = router_info_base64_ident(our_ri);
    i2p_info(LOG_ROUTER, "our router is %s", i);
    free(i);
  } else {
    // wtf? router info not loaded
    i2p_error(LOG_ROUTER, "failed to get our router info, was null");
    router_identity_free(&identity);
    return 0;
  }
  
  if(!i2p_netdb_ensure_skiplist(ctx->netdb)) {
    i2p_error(LOG_ROUTER, "failed to created netdb skiplist directory, %s", strerror(errno));
    return 0;
  }
  return 1;
}

void router_context_close(struct router_context * ctx)
{
  ntcp_server_detach(ctx->ntcp);
  // wait for ntcp disposal
}

static void router_init_netdb(struct router_context * ctx)
{
  if(!i2p_netdb_load_all(ctx->netdb)) {
    // no peers
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
              router_try_bootstrap_from_floodfill(ctx, ri);
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
      router_try_reseed_from(ctx, ctx->reseed);
    } else {
      i2p_error(LOG_ROUTER, "no floodfill to bootstrap from and reseed not enabled");
    }
  }
  
}
