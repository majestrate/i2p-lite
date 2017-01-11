#include <i2pd/chacha.h>
#include <sodium/crypto_aead_chacha20poly1305.h>


static int i2p_tunnel_v2_decrypt(struct tunnel_ChaCha * ch, tunnel_data_message_v2 * msg, tunnel_data_block_v2 * block)
{
  uint8_t * base = *msg;
  return  crypto_aead_chacha20poly1305_ietf_decrypt_detached(*block, NULL, // m / nsec
                                                             base+TUNDATA_V2_OFFSET, TUNDATA_V2_SIZE, // c / clen
                                                             base+TUNMAC_V2_OFFSET, // mac
                                                             base+TUNID_V2_OFFSET, TUNID_SIZE, // ad / adlen
                                                             base+TUNNONCE_V2_OFFSET, ch->key); // npub / k
}

static int i2p_tunnel_v2_encrypt(struct tunnel_ChaCha * ch, tunnel_data_block_v2 * block, tunnel_data_message_v2 * msg)
{
  uint8_t * base = *msg;
  return crypto_aead_chacha20poly1305_ietf_encrypt_detached(base+TUNDATA_V2_OFFSET, // c
                                                            base+TUNMAC_V2_OFFSET, NULL, // mac / maclen_p
                                                            *block, TUNDATA_V2_SIZE, // m / mlen
                                                            base+TUNID_V2_OFFSET, TUNID_SIZE, // ad / adlen
                                                            NULL, base+TUNNONCE_V2_OFFSET, // nsec / npub
                                                            ch->key); // k
}


void tunnel_ChaCha_init(struct tunnel_ChaCha * ch)
{
  ch->decrypt = i2p_tunnel_v2_decrypt;
  ch->encrypt = i2p_tunnel_v2_encrypt;
}
