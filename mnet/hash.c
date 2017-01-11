#include <mnet/hash.h>
#include <mnet/memory.h>
#include <sodium/crypto_generichash_blake2b.h>

struct mnet_hasher
{
  crypto_generichash_blake2b_state st;
};

void mnet_hash(ident_hash * h, const uint8_t * data, size_t sz)
{
  crypto_generichash_blake2b(*h, sizeof(ident_hash), data, sz, NULL, 0);
}

void mnet_hasher_new(struct mnet_hasher ** h)
{
  *h = xmalloc(sizeof(struct mnet_hasher));
  crypto_generichash_blake2b_init(&(*h)->st, NULL, 0, sizeof(ident_hash));
}

void mnet_hasher_free(struct mnet_hasher ** h)
{
  free(*h);
  *h = NULL;
}

void mnet_hasher_update(struct mnet_hasher * h, const uint8_t * data, size_t sz)
{
  crypto_generichash_blake2b_update(&h->st, data, sz);
}

void mnet_hasher_final(struct mnet_hasher * h, ident_hash * digest)
{
  crypto_generichash_blake2b_final(&h->st, *digest, sizeof(ident_hash));
}
