#ifndef I2PD_MEMORY_H_
#define I2PD_MEMORY_H_
#include <jemalloc/jemalloc.h>
#include "i2pendian.h"

#define xmalloc(sz) mallocx(sz, MALLOCX_ZERO)

uint16_t buf16toh(const void *buf);
 
uint32_t buf32toh(const void *buf);

uint64_t buf64toh(const void *buf);

uint16_t bufbe16toh(const void *buf);

uint32_t bufbe32toh(const void *buf);

uint64_t bufbe64toh(const void *buf);

void htobuf16(void *buf, uint16_t b16);

void htobuf32(void *buf, uint32_t b32);

void htobuf64(void *buf, uint64_t b64);

void htobe16buf(void *buf, uint16_t big16);

void htobe32buf(void *buf, uint32_t big32);

void htobe64buf(void *buf, uint64_t big64);


#endif
