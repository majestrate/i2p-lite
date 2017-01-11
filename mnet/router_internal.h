#ifndef I2PD_ROUTER_INTERNAL_H_
#define I2PD_ROUTER_INTERNAL_H_
#include <uv.h>
#include <i2pd/router.h>
#include <i2pd/transport.h>
#include <i2pd/tunnels.h>

#define ROUTER_CONTEXT_TICK_INTERVAL 5000

struct router_context
{

  // mainloop
  uv_loop_t * loop;

  // periodic event ticker
  uv_timer_t ticker;
  
  // base directory for data
  char * data_dir;
  // file for our router info
  char * router_info;
  // file for our router private keys
  char * router_keys;
  // file for floodfill boostrap, don't free
  const char * floodfill;
  // url for reseed, don't free
  const char * reseed;

  // ipv4 addr/port
  char * external_addr4;
  char * external_port4;
  // ipv6 addr/port
  char * external_addr6;
  char * external_port6;
  
  // network database storage
  struct i2p_netdb * netdb;  
  // transport layer
  struct i2np_transport * transport;
  // tunnel routing context
  struct i2np_tunnel_context * tunnels;
  // exploritory tunnel pool
  struct i2np_tunnel_pool * exploritory_pool;
  // ntcp server
  struct ntcp_server * ntcp;
  // our router identity private keys
  struct i2p_identity_keys * privkeys;
  // our router info
  struct router_info * our_ri;
  // our ident hash
  ident_hash ident;
};

#endif
