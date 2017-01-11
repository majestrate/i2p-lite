#ifndef I2PD_LEASE_H_
#define I2PD_LEASE_H_

#define V1_LEASE_SIZE 44
/** @brief network database lease */
typedef uint8_t i2p_v1_lease[V1_LEASE_SIZE];

// offset to tunnel id
#define V1_LEASE_TID_IDX 32

// offset to end date
#define V1_LEASE_EXPIRE_IDX 36

#endif
