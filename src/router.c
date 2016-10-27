#include <i2pd/router.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/transport.h>
#include <i2pd/util.h>

#include <string.h>

struct router_context
{

  i2p_filename router_info;
  i2p_filename router_keys;

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
  
  memcpy(&(*ctx)->router_info, &cfg.router_info, sizeof(i2p_filename));
  memcpy(&(*ctx)->router_keys, &cfg.router_keys, sizeof(i2p_filename));

  // init transports
  i2np_transport_new(&(*ctx)->transport);

  // alloc/configure ntcp
  ntcp_server_alloc(&(*ctx)->ntcp);
  ntcp_server_configure((*ctx)->ntcp, cfg.ntcp);

  // attach ntcp to transports
  ntcp_server_attach((*ctx)->ntcp, (*ctx)->transport);

  // init netdb parameters
  i2p_netdb_new(&(*ctx)->netdb, cfg.netdb);
  
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
  
  // free router context
  free(*ctx);
  *ctx = NULL;
}

int router_context_load(struct router_context * ctx)
{
  int ret = 1;
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
