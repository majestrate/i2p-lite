#ifndef I2PD_CRYPTO_H_
#define I2PD_CRYPTO_H_
#include <sodium/randombytes.h>

#define I2P_CONFIG_CRYPTO_CHECK "i2p.crypto.check"

struct i2p_crypto_config
{
  // do crypto sanity check
  int sanity_check;
};

/** @brief initialize crypto subsytem */
int i2p_crypto_init(struct i2p_crypto_config cfg);

/** @brief deinitialize crypto subsystem */
void i2p_crypto_done();

#endif
