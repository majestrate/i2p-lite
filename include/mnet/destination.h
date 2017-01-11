#ifndef I2PD_DESTINATION_H_
#define I2PD_DESTINATION_H_

struct i2p_destination;
typedef struct i2p_destination * destination_t;

/** @brief read a binary destination public key blob, return null if overflow, otherwise return where we stopped reading */
uint8_t * i2p_destination_read(destination_t * dest, uint8_t * in, size_t limit);


#endif
