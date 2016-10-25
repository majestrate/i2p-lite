#ifndef I2PD_LEASE_SET_H_
#define I2PD_LEASE_SET_H_
#include <i2pd/destination.h>

struct i2p_v1_leaseset;

typedef struct i2p_v1_leaseset * leseset_v1_t;


/** @brief read a full v1 lease set from buffer, return null if overflow, otherwise return where we stopped reading */
uint8_t * v1_leaseset_read(leaseset_v1_t * ls, uint8_t * in, size_t limit);

/** @brief return the size of this lease set or 0 if unitialized */
size_t v1_leaseset_size(leaseset_v1_t ls);

/** @brief write a lease set to buffer, return size written */
size_t v1_leaseset_write(leaseset_v1_t ls, uint8_t * out, size_t limit);

void v1_leaseset_sign(leaseset_v1_t ls, 

#endif
