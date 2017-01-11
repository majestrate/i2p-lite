#include <i2pd/rand.h>
#include <sodium/randombytes.h>

void i2p_rand(uint8_t * data, size_t sz)
{
  randombytes_buf(data, sz);
}

uint32_t i2p_random()
{
  return randombytes_random();
}
