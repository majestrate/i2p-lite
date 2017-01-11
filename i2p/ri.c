#include <i2pd/ri.h>
#include <i2pd/address.h>
#include <i2pd/encoding.h>
#include <i2pd/identity.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>
#include <i2pd/ntcp.h>
#include <i2pd/ssu.h>
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

int router_info_load(struct router_info * ri, FILE * f)
{
  return 0;
}

int router_info_verify(struct router_info * ri)
{
  int l = ri->len - i2p_identity_siglen(ri->identity);
  if(!l) i2p_error(LOG_DATA, "i2p router info has length of size 0");
  return l > 0 && i2p_identity_verify_data(ri->identity, ri->data, l, ri->data + l);
}

void router_info_generate(struct i2p_identity_keys * k, struct router_info_config * config, struct router_info ** info)
{
  router_info_new(info);
  i2p_identity_keys_to_public(k, &(*info)->identity);
  if(config->caps)
    (*info)->caps = strdup(config->caps);

  if(config->ntcp) {
    struct i2p_addr * addr = NULL;
    ntcp_config_to_address(config->ntcp, &addr);
    if(addr) {
      (*info)->addresses[(*info)->num_addresses] = addr;
      (*info)->num_addresses += 1;
    }
  }
  if(config->ssu) {
    struct i2p_addr * addr = NULL;
    ssu_config_to_address(config->ssu, &addr);
    if(addr) {
      (*info)->addresses[(*info)->num_addresses] = addr;
      (*info)->num_addresses += 1;
    }
  } 
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

void router_info_get_identity(struct router_info * ri, struct i2p_identity ** ident)
{
  *ident = ri->identity;
}

void router_info_config_new(struct router_info_config ** c)
{
  *c = xmalloc(sizeof(struct router_info_config));
}

void router_info_config_free(struct router_info_config ** c)
{
  if((*c)->ntcp) ntcp_config_free(&(*c)->ntcp);
  if((*c)->ssu) ssu_config_free(&(*c)->ssu);
  free((*c)->caps);
  free(*c);
  *c = NULL;
}
