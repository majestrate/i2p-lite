#ifndef I2PD_CERT_H_
#define I2PD_CERT_H_

#include <stdint.h>
#include <unistd.h>

#define I2P_CERT_TYPE_NULL 0
#define I2P_CERT_TYPE_HASHCASH 1
#define I2P_CERT_TYPE_HIDDEN 2
#define I2P_CERT_TYPE_SIGNED 3
#define I2P_CERT_TYPE_MULTIPLE 4
#define I2P_CERT_TYPE_KEY 5


struct i2p_cert;

uint8_t * i2p_cert_read(struct i2p_cert ** c, uint8_t * d, size_t len);

void i2p_cert_free(struct i2p_cert ** c);

int i2p_cert_type(struct i2p_cert * c);
size_t i2p_cert_data(struct i2p_cert * c, uint8_t ** d);

#endif
