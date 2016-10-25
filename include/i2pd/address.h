#ifndef I2PD_ADDRESS_H_
#define I2PD_ADDRESS_H_
#include "config.h"

/** @brief address to bind to */
struct i2p_bind_addr
{
  char host[MAX_HOST_SIZE];
  uint16_t port;
};


#endif
