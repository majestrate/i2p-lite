#ifndef I2PD_TRANSPORT_H_
#define I2PD_TRANSPORT_H_
#include <i2pd/i2np.h>
#include <i2pd/datatypes.h>
#include <i2pd/netdb.h>

#include <uv.h>

/** @brief transport implementation */
struct i2np_transport_impl;

/** @brief transport layer muxer */
struct i2np_transport;

/** @brief create new transport layer muxer with initialized uv_loop  */
void i2np_transport_new(struct i2np_transport ** t, uv_loop_t * loop);
void i2np_transport_free(struct i2np_transport ** t);

/** @brief recv next inbound i2np message from transport, returns -1 on error, otherwise returns 0 */
int i2np_recvfrom(struct i2np_transport * t, struct i2np_msg ** msg, ident_hash * from);

/** @brief send an i2np message to a router given its ident hash, returns -1 on error, otherwise returns 0 */
int i2np_sendto(struct i2np_transport * t, struct i2np_msg * msg, ident_hash to);

#endif
