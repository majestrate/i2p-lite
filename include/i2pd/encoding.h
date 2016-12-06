#ifndef I2PD_ENCODING_H_
#define I2PD_ENCODING_H_
#include <stdint.h>
#include <unistd.h>

size_t i2p_base32_encode(uint8_t * in, size_t ilen, char * out, size_t olen);

size_t i2p_base64_encode(uint8_t * in, size_t ilen, char * out, size_t olen);

size_t i2p_base64_decode(char * in, size_t ilen, uint8_t * out, size_t olen);

char * i2p_base64_encode_str(uint8_t * buf, size_t len);

size_t i2p_base64_decode_str(char * in, uint8_t ** out);

size_t i2p_base64_encoding_buffer_size(size_t input_size);

extern const char * I2P_BASE64_CHARS;

#endif
