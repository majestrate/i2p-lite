#ifndef I2PD_ROUTER_INTERNAL_H_
#define I2PD_ROUTER_INTERNAL_H_
#include <uv.h>
#include <i2pd/router.h>
#include <i2pd/transport.h>

struct router_context
{

  // mainloop
  uv_loop_t * loop;
  
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

#endif
