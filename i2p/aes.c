#include <i2pd/aes.h>
#include <i2pd/crypto.h>
#include <i2pd/memory.h>
#include <openssl/aes.h>
#include <string.h>

struct aes_key_impl
{
  aes_key native_key;
  AES_KEY openssl_key;
};

void aes_key_new(struct aes_key_impl ** k, aes_key * keydata)
{
  *k = xmalloc(sizeof(struct aes_key_impl));
  if(aesni_enabled()) {
    memcpy((*k)->native_key, *keydata, sizeof(aes_key));
  }
  AES_set_encrypt_key(*keydata, 256, &(*k)->openssl_key);
}

void aes_key_free(struct aes_key_impl ** k)
{
  free(*k);
  *k = NULL;
}

#if defined(__x86_64__) || defined(__SSE__)
#define XOR_BLOCK(a, b, c) __asm__("movups	(%[buf]), %%xmm0 \nmovups	(%[other]), %%xmm1 \npxor %%xmm1, %%xmm0 \nmovups	%%xmm0, (%[out]) \n": : [buf]"r"(c), [out]"r"(a),[other]"r"(b) : "%xmm0", "%xmm1", "memory")
#else
#define XOR_BLOCK(a, b, c) for(int _i=0; _i < 16; _i++) a[_i] = b[_i] ^ c[_i]
#endif


static void tunnel_aes_encrypt(struct tunnel_AES * aes, tunnel_data_message * data)
{
  uint8_t * iv = (*data) + 4; // block start + tunnelID
  // encrypt IV
  AES_encrypt(iv, iv, &aes->iv_key->openssl_key);
  uint8_t buf[16] = {0};
  uint8_t * prev_block = iv ;
  for (int i = 0; i < 64; i ++) {
    uint8_t * block = prev_block + 16;
    XOR_BLOCK(buf, prev_block, block);
    AES_encrypt(buf, block, &aes->layer_key->openssl_key);
    prev_block += 16;
  }
  // double encrypt IV
  AES_encrypt(iv, iv, &aes->iv_key->openssl_key);
}

static void tunnel_aes_decrypt(struct tunnel_AES * aes, tunnel_data_message * data)
{
  uint8_t * iv = (*data) + 4; // block start + tunnelID
  // decrypt iv
  AES_decrypt(iv, iv, &aes->iv_key->openssl_key);
  uint8_t buf[16]= {0};
  uint8_t * prev_block = iv ;
  for (int i = 0; i < 64; i ++) {
    uint8_t * block = prev_block + 16;
    AES_decrypt(block, buf, &aes->layer_key->openssl_key);
    XOR_BLOCK(buf, prev_block, block);
    prev_block += 16;
  }
  // double decrypt IV
  AES_decrypt(iv, iv, &aes->iv_key->openssl_key);
}


static void tunnel_AESNI_encrypt(struct tunnel_AES * aes, tunnel_data_message * block)
{
  // TODO: implement aesni assembler
  tunnel_aes_encrypt(aes, block);
}

static void tunnel_AESNI_decrypt(struct tunnel_AES * aes, tunnel_data_message * block)
{
  // TODO: implement aesni assembler
  tunnel_aes_decrypt(aes, block);
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
