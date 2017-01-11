#ifndef MNET_TUNNELS_H_
#define MNET_TUNNELS_H_
#include <mnet/garlic.h>
#include <mnet/netdb.h>
#include <mnet/tunnel_crypto.h>

// forward declare
struct router_context;


/** @brief handler of all inbound i2np messages for tunnels */
struct mnet_garlic_tunnel_context;

struct mnet_garlic_tunnel_hop
{
  struct router_info * ri;
  /** TODO: add remaining members */
};

/** @brief configuration for a tunnel build */
struct mnet_garlic_tunnel_config;

void mnet_garlic_tunnel_config_new(struct mnet_garlic_tunnel_config ** c);
void mnet_garlic_tunnel_config_free(struct mnet_garlic_tunnel_config ** c);
void mnet_garlic_tunnel_config_append_hop(struct mnet_garlic_tunnel_config * c, struct mnet_garlic_tunnel_hop h);

typedef void (*mnet_garlic_tunnel_config_iterator)(struct mnet_garlic_tunnel_config * , struct mnet_garlic_tunnel_hop *, void *);

void mnet_garlic_tunnel_config_for_each_hop(struct mnet_garlic_tunnel_config * c, mnet_garlic_tunnel_config_iterator i, void * user);

struct mnet_garlic_tunnel
{
  /** @brief parent context */
  struct mnet_garlic_tunnel_context * ctx;

  /** @brief 1 if inbound tunnel otherwise 0 (outbound tunnel) */
  uint8_t inbound;
  
  /** @brief status of this tunnel */
  uint8_t status;
  
  /** @brief this tunnel's tunnel id */
  tunnel_id_t our_tid;
  /** @brief next hop's tunnel id */
  tunnel_id_t next_tid;

  /**
     @brief tunnel encryption/decryption context
  */
  tunnel_Crypto_t crypto;
  
  /**
     @brief handle tunnel gateway message sent to this tunnel
  */
  void (*tunnel_gateway)(struct mnet_garlic_tunnel *, struct tgw_msg * );
  /**
     @brief handle version 2 tunnel data message sent to this tunnel
  */
  void (*tunnel_data)(struct mnet_garlic_tunnel *, tunnel_data_message_v2 *);

};

#define MNET_GARLIC_TUNNEL_STATUS_INITIAL (0)
#define MNET_GARLIC_TUNNEL_STATUS_BUILDING (1)
#define MNET_GARLIC_TUNNEL_STATUS_ESTABLISHED (2)
#define MNET_GARLIC_TUNNEL_STATUS_EXPIRING (3)
#define MNET_GARLIC_TUNNEL_STATUS_BUILD_TIMEOUT (4)
#define MNET_GARLIC_TUNNEL_STATUS_BUILD_REJECT (5)
#define MNET_GARLIC_TUNNEL_STATUS_BROKEN (6)


void mnet_garlic_tunnel_new(struct mnet_garlic_tunnel ** tunnel);
void mnet_garlic_tunnel_free(struct mnet_garlic_tunnel ** tunnel);;

struct mnet_garlic_tunnel_pool;

/** @brief peer selection hook for tunnel pool */
typedef int (*mnet_garlic_tunnel_pool_select_peers)(struct mnet_garlic_tunnel_pool*, struct mnet_netdb *, struct mnet_garlic_tunnel_config *, struct mnet_garlic_tunnel_config *);

/** @brief create new tunnel pool from tunnel context */
void mnet_garlic_tunnel_pool_new(struct mnet_garlic_tunnel_context * ctx, struct mnet_garlic_tunnel_pool ** pool);
void mnet_garlic_tunnel_pool_free(struct mnet_garlic_tunnel_pool ** pool);

/** @brief set peer selection hook, must be called before starting */
void mnet_garlic_tunnel_pool_set_peer_selector(mnet_garlic_tunnel_pool_select_peers inbound, mnet_garlic_tunnel_pool_select_peers outbound);

/** @brief run i2np tunnel pool, start building tunnel immediately */
void mnet_garlic_tunnel_pool_start(struct mnet_garlic_tunnel_pool * pool);

typedef void (*mnet_garlic_tunnel_pool_close_hook)(struct mnet_garlic_tunnel_pool *, void *);

/** @brief add a callback to be called when tunnel pool closes */
void mnet_garlic_tunnel_pool_add_close_hook(struct mnet_garlic_tunnel_pool * pool, mnet_garlic_tunnel_pool_close_hook h, void * user);

void mnet_garlic_tunnel_pool_stop(struct mnet_garlic_tunnel_pool * pool);

/** @brief select next outbound tunnel from pool */
void mnet_garlic_tunnel_pool_select_next_outbound_tunnel(struct mnet_garlic_tunnel_pool * pool, struct mnet_garlic_tunnel ** t);


void mnet_garlic_tunnel_context_new(struct router_context * router,  struct mnet_garlic_tunnel_context ** ctx);
void mnet_garlic_tunnel_context_free(struct mnet_garlic_tunnel_context ** ctx);

/** @brief get message router for handling inbound tunnel messages */
struct mnet_garlic_message_router * mnet_garlic_tunnel_context_message_router(struct mnet_garlic_tunnel_context * ctx);

/** @brief registers a tunnel pool onto a tunnel context, tunnel pool will be able to recv inbound messages after this */
void mnet_garlic_tunnel_context_register_pool(struct mnet_garlic_tunnel_context * ctx, struct mnet_garlic_tunnel_pool * pool);

#endif
