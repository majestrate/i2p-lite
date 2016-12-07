#include <i2pd/aes.h>
#include <i2pd/crypto.h>
#include <openssl/aes.h>
#include <string.h>

struct aes_key_impl
{
  aes_key native_key;
  AES_KEY openssl_key;
};

void aes_key_init(struct aes_key_impl * k, uint8_t * data)
{
  if(aesni_enabled()) {
    memcpy(k->native_key, data, sizeof(aes_key));
  } else {
    AES_set_encrypt_key(data, 256, &k->openssl_key);
  }
}


static void tunnel_AESNI_encrypt(struct tunnel_AES * aes, tunnel_data_message * block)
{
  // TODO: implement aesni assembler
}

static void tunnel_AESNI_decrypt(struct tunnel_AES * aes, tunnel_data_message * block)
{
  // TODO: implement aesni assembler
}

#if defined(__x86_64__) || defined(__SSE__)
#define XOR_BLOCK(b, a) __asm__("movups	(%[buf]), %%xmm0 \nmovups	(%[other]), %%xmm1 \npxor %%xmm1, %%xmm0 \nmovups	%%xmm0, (%[buf]) \n": : [buf]"r"(a), [other]"r"(b) : "%xmm0", "%xmm1", "memory")
#else
#define XOR_BLOCK(b, a) for(int _i=0; _i < 16; _i++) a[_i] ^= b[_i]
#endif


static void tunnel_aes_encrypt(struct tunnel_AES * aes, tunnel_data_message * block)
{
  uint8_t * iv = (*block) + 4; // block start + tunnelID
  // encrypt IV
  AES_ecb_encrypt(iv, iv, &aes->iv_key->openssl_key, AES_ENCRYPT);
  uint8_t * data = iv ;
  for (int i = 0; i < 64; i ++) {
    uint8_t * next = data + 16;
    XOR_BLOCK(data, next);
    AES_ecb_encrypt(next, next, &aes->layer_key->openssl_key, AES_ENCRYPT);
    data += 16;
  }
  // double encrypt IV
  AES_ecb_encrypt(iv, iv, &aes->iv_key->openssl_key, AES_ENCRYPT);
}

static void tunnel_aes_decrypt(struct tunnel_AES * aes, tunnel_data_message * block)
{
  uint8_t * iv = (*block) + 4; // block start + tunnelID
  // decrypt iv
  AES_ecb_encrypt(iv, iv, &aes->iv_key->openssl_key, AES_DECRYPT);
  uint8_t * data = iv ;
  for (int i = 0; i < 64; i ++) {
    uint8_t * next = data + 16;
    XOR_BLOCK(data, next);
    AES_ecb_encrypt(next, next, &aes->layer_key->openssl_key, AES_DECRYPT);
    data += 16;
  }
  // double decrypt IV
  AES_ecb_encrypt(iv, iv, &aes->iv_key->openssl_key, AES_DECRYPT);
}


void tunnel_AES_init(struct tunnel_AES * aes)
{
  if(aesni_enabled()) {
    // aesni enabled
    aes->encrypt = tunnel_AESNI_encrypt;
    aes->decrypt = tunnel_AESNI_decrypt;
  } else {
    aes->encrypt = tunnel_aes_encrypt;
    aes->decrypt = tunnel_aes_decrypt;
  }
}


int detect_aesni()
{
#ifdef __x86_64__
  unsigned int eax, ecx;
  const unsigned int flag = (1 << 25);
  __asm__ __volatile__(
    "cpuid"
    : "=a"(eax), "=c"(ecx)
    : "a"(1), "c"(0)
    : "%ebx", "%edx");
  return (ecx & flag) == flag;
#else
  return 0;
#endif
}
