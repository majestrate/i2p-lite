#ifndef I2PD_CHACHA_H_
#define I2PD_CHACHA_H_

#include <i2pd/datatypes.h>

/**
   @brief tunnel version 2 encryption/decryption context providing message authentication
 */
struct tunnel_ChaCha
{
  tunnel_key_t key;
  /**
     @brief the next hop's tunnel id
  */
  tunnel_id_t next_tid;

  /**
     @brief decrypt and verify tags for a version 2 tunnel data 
     @return -1 on tag verification failure or 0 if everything decrypted and verified correctly
  */
  int (*decrypt)(struct tunnel_ChaCha *, tunnel_data_message_v2 *, tunnel_data_block_v2 *);

  /**
     @brief encrypt a version 2 tunnel data block into a version 2 tunnel data message 
     @return -1 on fail otherwise return 0
   */
  int (*encrypt)(struct tunnel_ChaCha *, tunnel_data_block_v2 *, tunnel_data_message_v2 *);

};

/**
   @brief set function pointers on a struct tunnel_ChaCha
*/
void tunnel_ChaCha_init(struct tunnel_ChaCha * ch);

#endif
