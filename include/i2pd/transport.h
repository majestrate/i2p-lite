#ifndef I2PD_TRANSPORT_H_
#define I2PD_TRANSPORT_H_
#include <i2pd/i2np.h>
#include <i2pd/datatypes.h>

#include <uv.h>

/** @brief transport implementation */
struct i2np_transport_impl;

/** @brief transport layer muxer */
struct i2np_transport;

/** @brief create new transport layer muxer with initialized uv_loop */
void i2np_transport_new(struct i2np_transport ** t, uv_loop_t * loop);
void i2np_transport_free(struct i2np_transport ** t);

/** @brief send an i2np message to a router given its ident hash */
int i2np_sendto(struct i2np_transport * t, struct i2np_msg * msg, ident_hash * to);

/** @brief add a transport  to the muxer */
void i2np_transport_add_impl(struct i2np_transport * t, struct i2np_transport_impl * i);

/** @brief get event loop belonging to this transport */
uv_loop_t * i2np_transport_get_loop(struct i2np_transport * t);

/** @brief get cost to deliever a message */
int i2np_transport_impl_get_cost(struct i2np_transport_impl * i, struct i2np_msg * m);

#endif
