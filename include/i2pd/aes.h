#ifndef I2PD_AES_H_
#define I2PD_AES_H_
#include <stdint.h>
#include <stdlib.h>
#include <i2pd/datatypes.h>

/** @brief detect aesni on runtime, return 1 if we can use aesni otherwise return 0 */
int detect_aesni();

/** @brief aes iv buffer */
typedef uint8_t aes_iv[16];
typedef uint8_t aes_key[32];

/** @brief aes ecrypt/decrypt implementation context */
struct aes_key_impl;

/** @brief initialize aes key with key data */
void aes_key_new(struct aes_key_impl ** k, aes_key * key);

/** @brief free aes key */
void aes_key_free(struct aes_key_impl ** k);

/** @brief tunnel encryption / decryption context */
struct tunnel_AES
{
  struct aes_key_impl * layer_key;
  struct aes_key_impl * iv_key;
  void (*encrypt)(struct tunnel_AES *, tunnel_data_message*); // encrypt message in place
  void (*decrypt)(struct tunnel_AES *, tunnel_data_message*); // decrypt message in place
};

/** @breif initialize function pointers and members */
void tunnel_AES_init(struct tunnel_AES * aes);

struct ecb_AES
{
  struct aes_key_impl * key;
  void (*encrypt)(struct ecb_AES *, uint8_t *, size_t);
  void (*decrypt)(struct ecb_AES *, uint8_t *, size_t);
};

/** @brief initialize function pointers and members */
void ecb_AES_init(struct ecb_AES * aes);

struct cbc_AES
{
  struct aes_key_impl * key;
  aes_iv iv;
  void (*encrypt)(struct cbc_AES *, uint8_t *, size_t);
  void (*decrypt)(struct cbc_AES *, uint8_t *, size_t);
};

/** @brief initialize function pointers and members */
void cbc_AES_init(struct cbc_AES * aes);

#endif
