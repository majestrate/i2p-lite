#include <i2pd/tunnels.h>
#include <i2pd/memory.h>
#include <uv.h>

/** @brief maximum number of tunnels in a tunnel pool */
#define TUNNEL_POOL_MAX_TUNNELS 128

struct i2np_tunnel_pool
{
  /** @brief timer for building new tunnels, sending tunnel tests, expiring tunnels */
  uv_timer_t ticker;
  
  struct i2np_tunnel_context * ctx;
  tunnel_id_t tunnels[TUNNEL_POOL_MAX_TUNNELS]; // even slots are inbound tunnels, odd slots are outbound tunnels, 0 means no tunnel
};

void i2np_tunnel_pool_new(struct i2np_tunnel_context * ctx, struct i2np_tunnel_pool ** pool)
{
  *pool = xmalloc(sizeof(struct i2np_tunnel_pool));
  (*pool)->ctx = ctx;
}

void i2np_tunnel_pool_free(struct i2np_tunnel_pool ** pool)
{
  free(*pool);
  *pool = NULL;
}

void i2np_tunnel_pool_start(struct i2np_tunnel_pool * pool)
{

}

void i2np_tunnel_pool_stop(struct i2np_tunnel_pool * pool)
{
  
}
