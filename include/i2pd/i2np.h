#ifndef I2PD_I2NP_H_
#define I2PD_I2NP_H_

#include <i2pd/datatypes.h>

struct i2np_msg;

void i2np_msg_new(struct i2np_msg ** msg);
void i2np_msg_free(struct i2np_msg ** msg);

struct i2np_message_router_impl;

/** i2np message router dispatches inbound i2np messages that we get from i2np transport */
struct i2np_message_router
{
  struct i2np_message_router_impl * impl;
  void (*dispatch)(struct i2np_message_router_impl *, struct i2np_msg *, ident_hash); // dispatch an inbound message from a router by ident
};

#endif
