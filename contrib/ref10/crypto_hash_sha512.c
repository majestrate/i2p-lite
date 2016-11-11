#include "crypto_hash_sha512.h"
#include <openssl/sha.h>

void crypto_hash_sha512(unsigned char* output, const unsigned char* input, size_t len)
{
  SHA512(input, len, output);
}

void crypto_hash_sha512_2(unsigned char* out,
    const unsigned char* in1, size_t len1, 
    const unsigned char* in2, size_t len2)
{
  SHA512_CTX c;
  SHA512_Init(&c);
  SHA512_Update(&c, in1, len1);
  SHA512_Update(&c, in2, len2);
  SHA512_Final(out, &c);
}

void crypto_hash_sha512_3(unsigned char* out,
    const unsigned char* in1, size_t len1, 
    const unsigned char* in2, size_t len2,
    const unsigned char* in3, size_t len3
                          )
{
  SHA512_CTX c;
  SHA512_Init(&c);
  SHA512_Update(&c, in1, len1);
  SHA512_Update(&c, in2, len2);
  SHA512_Update(&c, in3, len3);
  SHA512_Final(out, &c);
}

