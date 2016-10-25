#ifndef I2PD_CRYPTO_H_
#define I2PD_CRYPTO_H_

/** @brief initialize crypto subsytem */
void i2p_crypto_init();

/** @brief do crypto sanity checks */
int i2p_crypto_test();

/** @brief deinitialize crypto subsystem */
void i2p_crypto_done();

#endif
