#ifndef I2PD_TRANSPORT_INTERNAL_H_
#define I2PD_TRANSPORT_INTERNAL_H_

#include <i2pd/transport.h>

#ifndef MAX_I2NP_TRANSPORTS 
#define MAX_I2NP_TRANSPORTS 3
#endif

struct i2np_transport
{
  uv_loop_t * loop;
  struct router_context * router;
  struct i2np_transport_impl links[MAX_I2NP_TRANSPORTS];
};




#endif
