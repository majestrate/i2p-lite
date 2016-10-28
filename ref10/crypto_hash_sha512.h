#ifndef CRYPTO_HASH_SHA512_H__
#define CRYPTO_HASH_SHA512_H__
#include <unistd.h>

void crypto_hash_sha512(unsigned char* output, const unsigned char* input, size_t len);
void crypto_hash_sha512_2(unsigned char* out,
    const unsigned char* in1, size_t len1, 
    const unsigned char* in2, size_t len2);
void crypto_hash_sha512_3(unsigned char* out,
    const unsigned char* in1, size_t len1, 
    const unsigned char* in2, size_t len2,
    const unsigned char* in3, size_t len3
                          );

#endif // CRYPTO_HASH_SHA512_H__
