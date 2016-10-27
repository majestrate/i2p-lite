#include <i2pd/router.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/transport.h>
#include <i2pd/util.h>

#include <string.h>
#include <sys/stat.h>

struct router_context
{

  // base directory for data
  char * data_dir;
  // file for our router info
  char * router_info;
  // file for our router private keys
  char * router_keys;

  // network database storage
  struct i2p_netdb * netdb;
  
  // transport layer
  struct i2np_transport * transport;
  // ntcp server
  struct ntcp_server * ntcp;
};


void router_context_new(struct router_context ** ctx, struct router_context_config cfg)
{
  *ctx = mallocx(sizeof(struct router_context), MALLOCX_ZERO);
  (*ctx)->data_dir = strdup(cfg.datadir);
  (*ctx)->router_info = path_join(cfg.datadir, "router.info", 0);
  (*ctx)->router_keys = path_join(cfg.datadir, "router.keys", 0);
  
  // init transports
  i2np_transport_new(&(*ctx)->transport);

  // alloc/configure ntcp
  ntcp_server_alloc(&(*ctx)->ntcp);
  ntcp_server_configure((*ctx)->ntcp, cfg.ntcp);

  // attach ntcp to transports
  ntcp_server_attach((*ctx)->ntcp, (*ctx)->transport);

  char * dir = path_join(cfg.datadir, "netDb", 0);
  // init netdb parameters
  i2p_netdb_new(&(*ctx)->netdb, dir);
  free(dir);
}

void router_context_free(struct router_context ** ctx)
{

  // detach ntcp
  ntcp_server_detach((*ctx)->ntcp);

  // free ntcp
  ntcp_server_free(&(*ctx)->ntcp);

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
  int ret = 1;

  if(!is_dir(ctx->data_dir)) {
    i2p_info(LOG_ROUTER, "creating data directory %s", ctx->data_dir);
    if(mkdir(ctx->data_dir, 0700) == -1) {
      i2p_error(LOG_ROUTER, "failed to create %s: %s", ctx->data_dir, strerror(errno));
      return 0;
    }
  }
  
  if(!check_file(ctx->router_keys)) {
    // generate router keys
    i2p_info(LOG_ROUTER, "generating new router identity at %s", ctx->router_keys);

    
  }
  if(!check_file(ctx->router_info)) {
    i2p_info(LOG_ROUTER, "saving router info at %s", ctx->router_info);

    
  }
  
  i2p_info(LOG_ROUTER, "load netdb");
  if(!i2p_netdb_load_all(ctx->netdb)) {
    i2p_error(LOG_ROUTER, "failed to load netdb");
  }
}


void router_context_run(struct router_context * ctx, uv_loop_t * loop)
{
  
}
