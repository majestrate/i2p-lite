#ifndef I2PD_TRANSPORT_H_
#define I2PD_TRANSPORT_H_
#include <i2pd/i2np.h>
#include <i2pd/datatypes.h>
#include <i2pd/netdb.h>

#include <uv.h>

/** @brief transport implementation details */
struct i2np_transport_impl
{
  void * impl; // implementation detail
  int (*sendto)(void*, struct i2np_msg *, ident_hash); // queue sending a message to a router by hash
  const char * name; // name of this transport
};

/** @brief transport layer muxer */
struct i2np_transport;

/** @brief create new transport layer muxer with initialized uv_loop and message router */
void i2np_transport_new(struct i2np_transport ** t, uv_loop_t * loop);
void i2np_transport_free(struct i2np_transport ** t);

/** @brief register transport impl with transport,  return 1 if we registered successfully otherwise return 0 */
int i2np_transport_register(struct i2np_transport * t, struct i2np_transport_impl * i);
/** @brief deregister transport impl with transport, does nothing if not registered */
void i2np_transport_deregister(struct i2np_transport * t, struct i2np_transport_impl * i);

/** @brief send an i2np message to a router given its ident hash, returns -1 on error, otherwise returns 0 */
int i2np_sendto(struct i2np_transport * t, struct i2np_msg * msg, ident_hash to);

#endif
