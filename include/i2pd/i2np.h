#ifndef I2PD_I2NP_H_
#define I2PD_I2NP_H_

#include <i2pd/datatypes.h>

struct i2np_msg;


void i2np_msg_free(struct i2np_msg ** msg);


/**
   @brief variable path build message
 */
struct vpb_msg;

void vpb_msg_new(struct vpb_msg ** msg);
void vpb_msg_free(struct vpb_msg ** msg);

typedef struct {
  struct router_info * target; // target
  tunnel_key_t key; // tunnel key
  tunnel_id_t recv_id; // tunnel id of recv tunnel
  tunnel_id_t next_id; // tunnel id of next tunnel
  ident_hash next_ident; // ident hash of next hop
  uint32_t lifetime; // desired tunnel lifetime, seconds
} build_record_t;

/**
   @brief add a build record to variable path build message
 */
void vpb_msg_put_hop(struct vpb_msg * msg, build_record_t cfg);

/**
   @brief variable path cancel message
   message for premature path canceling
 */
struct vpc_msg;

void vpc_msg_new(struct vpc_msg ** msg);
void vpc_msg_free(struct vpc_msg ** msg);

/**
   @brief database store message
 */
struct dbs_msg;

void dbs_msg_new(struct dbs_msg ** msg);
void dbs_msg_free(struct dbs_msg ** msg);

/**
   @brief tunnel gateway message
 */
struct tgw_msg;

void tgw_msg_new(struct tgw_msg ** msg);
void tgw_msg_free(struct tgw_msg ** msg);

/** i2np message router dispatches inbound i2np messages that we get from i2np transport */
struct i2np_message_router
{
  void * impl;
  void (*vpbm)(void *, struct vpb_msg *, ident_hash);
  void (*tdmv2)(void *, tunnel_data_message_v2 *, ident_hash);
  void (*vpcm)(void *, struct vpc_msg *, ident_hash);
  void (*tgwm)(void *, struct tgw_msg *, ident_hash);
  void (*dbsm)(void *, struct dbs_msg *, ident_hash);
};


#define DI_SIZE (sizeof(uint16_t) + sizeof(ident_hash) + sizeof(sym_key_t) + sizeof(tunnel_id_t) + sizeof(uint16_t))

/**
   @brief delivery instrctions for garlic cloves
   2 bytes flags
   32 bytes ident hash
   32 bytes symmettric key
   4 byte big endian unsigned integer tunnel id
   2 byte big endian unsigned integer delay (milliseconds)
*/
typedef uint8_t delivery_instructions[DI_SIZE];



#endif
