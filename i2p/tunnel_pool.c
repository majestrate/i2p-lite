#include <i2pd/tunnels.h>
#include <i2pd/memory.h>

/** @brief maximum number of tunnels in a tunnel pool */
#define TUNNEL_POOL_MAX_TUNNELS 128

struct i2np_tunnel_pool
{
  struct i2np_tunnel_context * ctx;
  tunnel_id_t tunnels[TUNNEL_POOL_MAX_TUNNELS]; // even slots are inbound tunnels, odd slots are outbound tunnels, 0 means no tunnel
};

void i2np_tunnel_pool_new(struct i2np_tunnel_pool ** pool)
{
  *pool = xmalloc(sizeof(struct i2np_tunnel_pool));
}
