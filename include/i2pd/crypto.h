#ifndef I2PD_CRYPTO_H_
#define I2PD_CRYPTO_H_

#define I2P_CONFIG_CRYPTO_CHECK "i2p.crypto.check"

struct i2p_crypto_config
{
  // do crypto sanity check
  int sanity_check;
  // enable aesni if possible
  int aesni;
};

#define default_crypto_config { 1, 0 }

struct crypto_consts;

extern struct crypto_consts * crypto;

/** return 1 if aesni is enabled on runtime */
int aesni_enabled();

/** @brief initialize crypto subsytem */
int i2p_crypto_init(struct i2p_crypto_config cfg);

/** @brief deinitialize crypto subsystem */
void i2p_crypto_done();

#endif
