#include <i2pd/ri.h>
#include <i2pd/identity.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>
#include <openssl/sha.h>

#include <sys/stat.h>
#include <errno.h>

#define MAX_ROUTER_INFO_SIZE 4096

struct router_info
{
  uint8_t data[MAX_ROUTER_INFO_SIZE];
  size_t len;
  struct i2p_identity * identity;  
};

void router_info_new(struct router_info ** ri)
{
  (*ri) = mallocx(sizeof(struct router_info), MALLOCX_ZERO);
}

void router_info_free(struct router_info ** ri)
{
  i2p_identity_free(&(*ri)->identity);
  free(*ri);
}

int router_info_load(struct router_info * ri, int fd)
{
  int ret;
  struct stat st;
  if(fstat(fd, &st) == -1) {
    i2p_error(LOG_DATA, "fstat %s", strerror(errno));
    return 0;
  }
  ri->len = st.st_size;
  
  if(ri->len > sizeof(ri->data)) return 0; // overflow

  size_t idx = 0;
  ssize_t r = 0;
  do {
    r = read(fd, ri->data + idx, 128);
    if (r == -1) break;
    idx += r;
  } while(idx < ri->len);

  ret = idx == ri->len;
  
  if (!ret)  {
    // bad read
    ri->len = 0;
    i2p_error(LOG_DATA, "failed to load router info, short read");
  }
  if(i2p_identity_read(&ri->identity, ri->data, ri->len)) {
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

int router_info_write(struct router_info * ri, int fd)
{
  if(!ri->len) return 0; // empty
  int r = write(fd, ri->data, ri->len);
  if(r == -1) return 0;
  if(fsync(fd) == -1) return 0;
  return r == ri->len;
}

void router_info_hash(struct router_info * ri, ident_hash * ident)
{
  i2p_identity_hash(ri->identity, ident);
}
