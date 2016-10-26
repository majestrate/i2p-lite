#ifndef I2PD_ENCODING_H_
#define I2PD_ENCODING_H_
#include <stdint.h>
#include <unistd.h>

void i2p_base32_encode(char * out, size_t olen, uint8_t * in, size_t ilen);

#endif
