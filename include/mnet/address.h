#ifndef MNET_ADDRESS_H_
#define MNET_ADDRESS_H_
#include <mnet/datatypes.h>
#include <mnet/types.h>
#include <unistd.h>


/** @brief network address */
struct mnet_addr
{
  char * style;
  char * host;
  uint16_t port;
  uint8_t cost;
  uint64_t date;
  pub_enc_key_t key;
};

/** @brief read addr from memory */
uint8_t * mnet_addr_read_dict(struct mnet_addr ** addr, uint8_t * b, size_t l);
/** @brief free an addr */
void mnet_addr_free(struct mnet_addr ** addr);

/** @brief get string represenation of port, caller must free return value*/
char * mnet_addr_port_str(struct mnet_addr * addr);

#endif
