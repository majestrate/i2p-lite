#ifndef MNET_CRYPTO_H_
#define MNET_CRYPTO_H_

#define MNET_CONFIG_CRYPTO_CHECK "i2p.crypto.check"

struct mnet_crypto_config
{
  // do crypto sanity check
  int sanity_check;
  // enable aesni if possible
  int aesni;
};

#define default_crypto_config { 1, 0 }

/** @brief initialize crypto subsytem */
int mnet_crypto_init(struct mnet_crypto_config cfg);

/** @brief deinitialize crypto subsystem */
void mnet_crypto_done();

#endif
