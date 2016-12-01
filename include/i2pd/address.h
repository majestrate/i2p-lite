#ifndef I2PD_ADDRESS_H_
#define I2PD_ADDRESS_H_
#include <i2pd/types.h>
#include <unistd.h>

/** @brief network address */
struct i2p_addr
{
  // what transport type (unused when binding)
  char * style;
  char * host;
  uint16_t port;
  uint8_t cost;
  uint64_t date;
};

/** @brief read i2p addr from memory */
uint8_t * i2p_addr_read_dict(struct i2p_addr ** addr, uint8_t * b, size_t l);
/** @brief free an i2p addr */
void i2p_addr_free(struct i2p_addr ** addr);

/** @brief get string represenation of port, caller must free return value*/
char * i2p_addr_port_str(struct i2p_addr * addr);

#endif
