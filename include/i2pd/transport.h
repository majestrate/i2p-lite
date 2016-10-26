#ifndef I2PD_TRANSPORT_H_
#define I2PD_TRANSPORT_H_
#include <i2pd/i2np.h>
#include <i2pd/datatypes.h>

/** @brief transport implementation */
struct i2np_transport_impl;

/** @brief transport layer muxer */
struct i2np_transport;


void i2np_transport_new(struct i2np_transport ** t);
void i2np_transport_free(struct i2np_transport ** t);

/** @brief send an i2np message to a router given its ident hash */
int i2np_sendto(struct i2np_transport * t, struct i2np_msg * msg, ident_hash * to);

/** @brief add a transport  to the muxer */
void i2np_transport_add_impl(struct i2np_transport * t, struct i2np_transport_impl * i);

/** @brief get cost to deliever a message */
int i2np_transport_impl_get_cost(struct i2np_transport_impl * i, struct i2np_msg * m);

#endif
