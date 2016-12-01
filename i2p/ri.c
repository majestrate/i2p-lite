#include <i2pd/ri.h>
#include <i2pd/address.h>
#include <i2pd/encoding.h>
#include <i2pd/identity.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>
#include <i2pd/util.h>
#include <openssl/sha.h>

#include <sys/stat.h>
#include <errno.h>

struct router_info
{
  uint8_t * data;
  size_t len;
  struct i2p_identity * identity;

  uint64_t timestamp;

  uint8_t num_addresses;
  struct i2p_addr ** addresses;
  
  char * caps;
};

void router_info_new(struct router_info ** ri)
{
  (*ri) = mallocx(sizeof(struct router_info), MALLOCX_ZERO);
}

void router_info_free(struct router_info ** ri)
{
  i2p_identity_free(&(*ri)->identity);
  size_t n = (*ri)->num_addresses;
  while(n--)
    i2p_addr_free(&(*ri)->addresses[n]);
  free((*ri)->data);
  free((*ri)->caps);
  free(*ri);
}

void router_info_process_props(char * k, char * v, void * u)
{
  struct router_info * ri = (struct router_info *) u;
  
  if(!strcmp(k, "caps")) ri->caps = strdup(v);
  
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

  ri->data = mallocx(ri->len, MALLOCX_ZERO);

  uint8_t * d;
  
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
  i2p_identity_new(&ri->identity);
  if(( d = i2p_identity_read_buffer(ri->identity, ri->data, ri->len))) {
    if (i2p_identity_sigtype(ri->identity)) 
      ret = router_info_verify(ri);
    else // dsa not supported
      return 0;
    if(ret) {
      // parse internal members
      // read timestamp
      ri->timestamp = bufbe64toh(d);
      d += sizeof(uint64_t);
      // read addresses
      uint8_t num = *d;
      i2p_debug(LOG_DATA, "router info has %d addresses", num);
      ri->num_addresses = num;
      d ++;
      if (num) {
        ri->addresses = mallocx(ri->num_addresses * sizeof(struct i2p_addr *), MALLOCX_ZERO);
        uint8_t i = 0;
        while(i < num && d) {
          d = i2p_addr_read_dict( &(ri->addresses[i]), d, d - ri->data);
          i ++;
        }
      }
      // peers
      uint8_t numPeers = *d;
      // skip peers
      d += numPeers * 32 + 1;
      // read properties
      d = read_i2pdict(d, d - ri->data, router_info_process_props, ri);
    } else {
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

void router_info_iter_addrs(struct router_info * ri, router_info_addr_iter i, void * u)
{
  uint8_t num = ri->num_addresses;
  uint8_t idx = 0;
  while(idx < num)
    i(ri, ri->addresses[idx++], u);
}

char * router_info_base64_ident(struct router_info * ri)
{
  ident_hash h = {0};
  char s[128] = {0};
  router_info_hash(ri, &h);
  i2p_base64_encode(h, sizeof(ident_hash), s, sizeof(s));
  return strdup(s);
}

void router_info_hash(struct router_info * ri, ident_hash * ident)
{
  i2p_identity_hash(ri->identity, ident);
}

int router_info_is_floodfill(struct router_info * ri)
{
  int ret = 0;
  if(ri->caps) {
    char * cap = ri->caps;
    while(*cap) {
      if(*cap == 'f') ret = 1;
      cap++;
    }
  }
  return ret;
}

void router_info_get_caps(struct router_info * ri, char ** caps)
{
  if(ri->caps) *caps = strdup(ri->caps);
  else *caps = NULL;
}
