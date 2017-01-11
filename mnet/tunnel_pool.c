#include <mnet/tunnels.h>
#include <mnet/memory.h>
#include <uv.h>

/** @brief maximum number of tunnels in a tunnel pool */
#define TUNNEL_POOL_MAX_TUNNELS 128

struct mnet_garlic_tunnel_pool
{
  /** @brief timer for building new tunnels, sending tunnel tests, expiring tunnels */
  uv_timer_t ticker;
  
  struct mnet_garlic_tunnel_context * ctx;
  tunnel_id_t tunnels[TUNNEL_POOL_MAX_TUNNELS]; // even slots are inbound tunnels, odd slots are outbound tunnels, 0 means no tunnel
};

void mnet_garlic_tunnel_pool_new(struct mnet_garlic_tunnel_context * ctx, struct mnet_garlic_tunnel_pool ** pool)
{
  *pool = xmalloc(sizeof(struct mnet_garlic_tunnel_pool));
  (*pool)->ctx = ctx;
}

void mnet_garlic_tunnel_pool_free(struct mnet_garlic_tunnel_pool ** pool)
{
  free(*pool);
  *pool = NULL;
}

void mnet_garlic_tunnel_pool_start(struct mnet_garlic_tunnel_pool * pool)
{

}

void mnet_garlic_tunnel_pool_stop(struct mnet_garlic_tunnel_pool * pool)
{
  
}
