#ifndef I2PD_TUNNELS_H_
#define I2PD_TUNNELS_H_
#include <i2pd/i2np.h>


struct i2np_transit_tunnel_muxer;

struct i2np_tunnel_pool_muxer;

struct i2np_tunnel_pool;

struct i2np_tunnel;

struct i2np_tunnel_context;

void i2np_tunnel_context_new(struct i2np_tunnel_context ** ctx);
void i2np_tunnel_context_free(struct i2np_tunnel_context ** ctx);

/** @brief get message router for handling inbound tunnel messages */
struct i2np_message_router * i2np_tunnel_context_message_router(struct i2np_tunnel_context * ctx);

#endif
