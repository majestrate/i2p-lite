#include <i2pd/ri.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>
#include <openssl/sha.h>

struct router_info
{
  uint8_t * data;
  size_t len;
  struct i2p_identity * identity;
  
};

void router_info_new(struct router_info ** ri)
{
  (*ri) = mallocx(sizeof(struct router_info), MALLOCX_ZERO);
}

void router_info_free(struct router_info ** ri)
{
  free((*ri)->data);
  free(*ri);
  *ri = NULL;
}

int router_info_load(struct router_info * ri, FILE * f)
{
  uint8_t * d;
  int ret;
  fseek(f, 0, SEEK_END);
  ri->len = ftell(f);
  rewind(f);
  ri->data = malloc(ri->len);
  ret = fread(ri->data, ri->len, 1, f);
  if (!ret)  {
    // bad read
    free(ri->data);
    ri->data = NULL;
    ri->len = 0;
    i2p_error(LOG_DATA, "failed to load router info, short read");
  }
  d = i2p_identity_read(&ri->identity, ri->data, ri->len);
  if(d) {
    ret = router_info_verify(ri);
    if(!ret) {
      i2p_error(LOG_DATA, "router info has invalid siganture");
    }
  } else {
    i2p_error(LOG_DATA, "failed to read identity for router info of size %lu", ri->len);
    // bad router identitiy
    ret = 0;
  }
  return ret;
}
int router_info_verify(struct router_info * ri)
{
  int l = ri->len - i2p_identity_siglen(ri->identity);
  return l > 0 && i2p_identity_verify_data(ri->identity, ri->data, l, ri->data + l);
}

int router_info_write(struct router_info * ri, FILE * f)
{
  if(!ri->len) return 0; // empty
  return fwrite(ri->data, ri->len, 1, f) != 0;
}

void router_info_calculate_hash(struct router_info * ri, ident_hash * ident)
{
  SHA256(ri->data, ri->len, *ident);
}
