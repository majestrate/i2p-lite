#include <i2pd/hash.h>
#include <i2pd/memory.h>
#include <sodium/crypto_generichash_blake2b.h>

struct i2p_hasher
{
  crypto_generichash_blake2b_state st;
};

void i2p_hash(ident_hash * h, const uint8_t * data, size_t sz)
{
  crypto_generichash_blake2b(*h, sizeof(ident_hash), data, sz, NULL, 0);
}

void i2p_hasher_new(struct i2p_hasher ** h)
{
  *h = xmalloc(sizeof(struct i2p_hasher));
  crypto_generichash_blake2b_init(&(*h)->st, NULL, 0, sizeof(ident_hash));
}

void i2p_hasher_free(struct i2p_hasher ** h)
{
  free(*h);
  *h = NULL;
}

void i2p_hasher_update(struct i2p_hasher * h, const uint8_t * data, size_t sz)
{
  crypto_generichash_blake2b_update(&h->st, data, sz);
}

void i2p_hasher_final(struct i2p_hasher * h, ident_hash * digest)
{
  crypto_generichash_blake2b_final(&h->st, *digest, sizeof(ident_hash));
}
