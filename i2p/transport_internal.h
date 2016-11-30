#ifndef I2PD_TRANSPORT_INTERNAL_H_
#define I2PD_TRANSPORT_INTERNAL_H_

#include <i2pd/transport.h>

struct i2np_transport
{
  uv_loop_t * loop;
  struct router_context * router;
};




#endif
