#ifndef MNET_ROUTER_INTERNAL_H_
#define MNET_ROUTER_INTERNAL_H_
#include <uv.h>
#include <mnet/router.h>
#include <mnet/transport.h>
#include <mnet/tunnels.h>

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
  // file for node boostrap, don't free
  const char * bootstrap;

  // ipv4 addr/port
  char * external_addr4;
  char * external_port4;
  // ipv6 addr/port
  char * external_addr6;
  char * external_port6;
  
  // network database storage
  struct mnet_netdb * netdb;  
  // transport layer
  struct mnet_garlic_transport * transport;
  // tunnel routing context
  struct mnet_garlic_tunnel_context * tunnels;
  // exploritory tunnel pool
  struct mnet_garlic_tunnel_pool * exploritory_pool;
  // our router identity private keys
  struct mnet_identity_keys * privkeys;
  // our router info
  struct router_info * our_ri;
  // our ident hash
  ident_hash ident;
};

#endif
