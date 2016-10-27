#ifndef I2PD_RI_H_
#define I2PD_RI_H_
#include <i2pd/datatypes.h>

#include <stdint.h>

struct router_info;

void router_info_new(struct router_info ** ri);
void router_info_free(struct router_info ** ri);

/** @brief load router info from open file and verify */
int router_info_load(struct router_info * ri, int fd);

/** @brief verify router info signature */
int router_info_verify(struct router_info * ri);

/** @brief read router info from buffer, return NULL on overflow otherwise return address in buffer were we stopped reading */
uint8_t * router_info_load_buffer(struct router_info * ri, uint8_t * buf, size_t len);

/** @brief calculate ident hash of router info */
void router_info_hash(struct router_info * ri, ident_hash * ident);

/** @brief write router info to file */
int router_info_write(struct router_info * ri, int fd);

#endif
