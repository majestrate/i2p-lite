#include <mnet/tunnels.h>
#include <mnet/memory.h>

void mnet_garlic_tunnel_new(struct mnet_garlic_tunnel ** tunnel)
{
  *tunnel = xmalloc(sizeof(struct mnet_garlic_tunnel));
}

void mnet_garlic_tunnel_free(struct mnet_garlic_tunnel ** tunnel)
{
  free(*tunnel);
  *tunnel = NULL;
}
