#ifndef I2PD_ADDRESS_H_
#define I2PD_ADDRESS_H_
#include <i2pd/types.h>

/** @brief network address */
struct i2p_addr
{
  // what transport type (unused when binding)
  char style [5];
  i2p_hostname host;
  uint16_t port;
  uint8_t cost;
  uint64_t date;
};

#endif
