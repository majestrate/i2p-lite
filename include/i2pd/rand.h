#ifndef I2PD_RAND_H_
#define I2PD_RAND_H_
#include <stdint.h>
#include <stdlib.h>

void i2p_rand(uint8_t * data, size_t sz);

uint32_t i2p_random();

#endif
