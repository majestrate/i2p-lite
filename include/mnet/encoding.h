#ifndef MNET_ENCODING_H_
#define MNET_ENCODING_H_
#include <stdint.h>
#include <unistd.h>

size_t mnet_base32_encode(uint8_t * in, size_t ilen, char * out, size_t olen);

size_t mnet_base64_encode(uint8_t * in, size_t ilen, char * out, size_t olen);

size_t mnet_base64_decode(char * in, size_t ilen, uint8_t * out, size_t olen);

char * mnet_base64_encode_str(uint8_t * buf, size_t len);

size_t mnet_base64_decode_str(char * in, uint8_t ** out);

size_t mnet_base64_encoding_buffer_size(size_t input_size);

extern const char * MNET_BASE64_CHARS;

#endif
