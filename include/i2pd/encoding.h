#ifndef I2PD_ENCODING_H_
#define I2PD_ENCODING_H_
#include <stdint.h>
#include <unistd.h>

size_t i2p_base32_encode(uint8_t * in, size_t ilen, char * out, size_t olen);

size_t i2p_base64_encode(uint8_t * in, size_t ilen, char * out, size_t olen);

#endif
