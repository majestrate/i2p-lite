#ifndef I2PD_I2NP_H_
#define I2PD_I2NP_H_

#include <i2pd/datatypes.h>

struct i2np_msg;


void i2np_msg_free(struct i2np_msg ** msg);




/** @brief get i2np messgae type */
uint8_t i2np_msg_type(struct i2np_msg * msg);

/** i2np message router dispatches inbound i2np messages that we get from i2np transport */
struct i2np_message_router
{
  void * impl;
  void (*dispatch)(void *, struct i2np_msg *, ident_hash); // dispatch an inbound message from a router by ident
};

/** @brief delivery instrctions for garlic cloves */
typedef uint8_t garlic_delivery_instructions[37];

#endif
