#ifndef I2PD_TUNNELS_H_
#define I2PD_TUNNELS_H_
#include <i2pd/i2np.h>
#include <i2pd/netdb.h>
#include <i2pd/aes.h>

// forward declare
struct router_context;


/** @brief handler of all inbound i2np messages for tunnels */
struct i2np_tunnel_context;

/** @brief tunnel id type, a 4 byte integer */
typedef uint32_t tunnel_id_t;

struct i2np_tunnel_hop
{
  struct router_info * ri;
  /** TODO: add remaining members */
};

/** @brief configuration for a tunnel build */
struct i2np_tunnel_config;

void i2np_tunnel_config_new(struct i2np_tunnel_config ** c);
void i2np_tunnel_config_free(struct i2np_tunnel_config ** c);
void i2np_tunnel_config_append_hop(struct i2np_tunnel_config * c, struct i2np_tunnel_hop h);

typedef void (*i2np_tunnel_config_iterator)(struct i2np_tunnel_config * , struct i2np_tunnel_hop *, void *);

void i2np_tunnel_config_for_each_hop(struct i2np_tunnel_config * c, i2np_tunnel_config_iterator i, void * user);

struct i2np_tunnel
{
  /** @brief parent context */
  struct i2np_tunnel_context * ctx;

  /** @brief 1 if inbound tunnel otherwise 0 (outbound tunnel) */
  uint8_t inbound;
  
  /** @brief status of this tunnel */
  uint8_t status;
  
  /** @brief this tunnel's tunnel id */
  tunnel_id_t our_tid;
  /** @brief next hop's tunnel id */
  tunnel_id_t next_tid;

  /** @brief tunnel encrypt/decrypt */
  struct tunnel_AES aes;
  
  /** @brief handle tunnel gateway message sent to this tunnel */
  void (*tunnel_gateway)(struct i2np_tunnel *, struct i2np_msg * );
  /** @brief handle tunnel data message sent to this tunnel, either encrypts or decrypts */
  void (*tunnel_data)(struct i2np_tunnel *,tunnel_data_message *);
  
};

#define I2NP_TUNNEL_STATUS_INITIAL (0)
#define I2NP_TUNNEL_STATUS_BUILDING (1)
#define I2NP_TUNNEL_STATUS_ESTABLISHED (2)
#define I2NP_TUNNEL_STATUS_EXPIRING (3)
#define I2NP_TUNNEL_STATUS_BUILD_TIMEOUT (4)
#define I2NP_TUNNEL_STATUS_BUILD_REJECT (5)
#define I2NP_TUNNEL_STATUS_BROKEN (6)


void i2np_tunnel_new(struct i2np_tunnel ** tunnel);
void i2np_tunnel_free(struct i2np_tunnel ** tunnel);;

struct i2np_tunnel_pool;

/** @brief peer selection hook for tunnel pool */
typedef int (*i2np_tunnel_pool_select_peers)(struct i2np_tunnel_pool*, struct i2p_netdb *, struct i2np_tunnel_config *, struct i2np_tunnel_config *);

/** @brief create new tunnel pool from tunnel context */
void i2np_tunnel_pool_new(struct i2np_tunnel_context * ctx, struct i2np_tunnel_pool ** pool);
void i2np_tunnel_pool_free(struct i2np_tunnel_pool ** pool);

/** @brief set peer selection hook, must be called before starting */
void i2np_tunnel_pool_set_peer_selector(i2np_tunnel_pool_select_peers inbound, i2np_tunnel_pool_select_peers outbound);

/** @brief run i2np tunnel pool, start building tunnel immediately */
void i2np_tunnel_pool_start(struct i2np_tunnel_pool * pool);

typedef void (*i2np_tunnel_pool_close_hook)(struct i2np_tunnel_pool *, void *);

/** @brief add a callback to be called when tunnel pool closes */
void i2np_tunnel_pool_add_close_hook(struct i2np_tunnel_pool * pool, i2np_tunnel_pool_close_hook h, void * user);

void i2np_tunnel_pool_stop(struct i2np_tunnel_pool * pool);

/** @brief select next outbound tunnel from pool */
void i2np_tunnel_pool_select_next_outbound_tunnel(struct i2np_tunnel_pool * pool, struct i2np_tunnel ** t);


void i2np_tunnel_context_new(struct router_context * router,  struct i2np_tunnel_context ** ctx);
void i2np_tunnel_context_free(struct i2np_tunnel_context ** ctx);

/** @brief get message router for handling inbound tunnel messages */
struct i2np_message_router * i2np_tunnel_context_message_router(struct i2np_tunnel_context * ctx);

/** @brief registers a tunnel pool onto a tunnel context, tunnel pool will be able to recv inbound messages after this */
void i2np_tunnel_context_register_pool(struct i2np_tunnel_context * ctx, struct i2np_tunnel_pool * pool);

#endif
