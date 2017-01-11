#ifndef MNET_CERT_H_
#define MNET_CERT_H_

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#define I2P_CERT_TYPE_NULL 0
#define I2P_CERT_TYPE_HASHCASH 1
#define I2P_CERT_TYPE_HIDDEN 2
#define I2P_CERT_TYPE_SIGNED 3
#define I2P_CERT_TYPE_MULTIPLE 4
#define I2P_CERT_TYPE_KEY 5


struct mnet_cert;

void mnet_cert_new(struct mnet_cert ** c);
/** @brief initialize from data */
void mnet_cert_init(struct mnet_cert * c, uint8_t type, uint8_t * data, uint16_t len);

/** @brief read from file descriptor */
int mnet_cert_read(struct mnet_cert * c, FILE * f);

/** @brief write to file descriptor */
int mnet_cert_write(struct mnet_cert * c, FILE * f);

/** @brief read from buffer */
uint8_t * mnet_cert_read_buffer(struct mnet_cert * c, uint8_t * d, size_t len);

void mnet_cert_free(struct mnet_cert ** c);

uint8_t mnet_cert_type(struct mnet_cert * c);

uint8_t * mnet_cert_buffer(struct mnet_cert * c);
uint16_t mnet_cert_buffer_length(struct mnet_cert * c);
uint8_t * mnet_cert_data(struct mnet_cert * c);
uint16_t mnet_cert_data_length(struct mnet_cert * c);

#endif
