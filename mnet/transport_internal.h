#ifndef MNET_TRANSPORT_INTERNAL_H_
#define MNET_TRANSPORT_INTERNAL_H_

#include <mnet/transport.h>

#ifndef MAX_GARLIC_TRANSPORTS 
#define MAX_GARLIC_TRANSPORTS 3
#endif

struct mnet_garlic_transport
{
  uv_loop_t * loop;
  struct router_context * router;
  struct mnet_garlic_transport_impl links[MAX_GARLIC_TRANSPORTS];
};




#endif
