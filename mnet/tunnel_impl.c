#include <i2pd/tunnels.h>
#include <i2pd/memory.h>

void i2np_tunnel_new(struct i2np_tunnel ** tunnel)
{
  *tunnel = xmalloc(sizeof(struct i2np_tunnel));
}

void i2np_tunnel_free(struct i2np_tunnel ** tunnel)
{
  free(*tunnel);
  *tunnel = NULL;
}
