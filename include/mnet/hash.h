#ifndef MNET_HASH_H_
#define MNET_HASH_H_
#include <mnet/datatypes.h>




void mnet_hash(ident_hash * h, const uint8_t * data, size_t sz);

struct mnet_hasher;

void mnet_hasher_new(struct mnet_hasher ** h);
void mnet_hasher_free(struct mnet_hasher ** h);

void mnet_hasher_update(struct mnet_hasher * h, const uint8_t * data, size_t len);
void mnet_hasher_final(struct mnet_hasher * h, ident_hash * digest);

#endif
