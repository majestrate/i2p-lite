#include <mnet/ri.h>
#include <mnet/address.h>
#include <mnet/encoding.h>
#include <mnet/identity.h>
#include <mnet/memory.h>
#include <mnet/log.h>
#include <mnet/iwp.h>
#include <mnet/util.h>

#include <sys/stat.h>
#include <errno.h>

struct router_info
{
  uint8_t * data;
  size_t len;
  struct mnet_identity * identity;

  uint64_t timestamp;

  uint8_t num_addresses;
  struct mnet_addr ** addresses;
  
  char * caps;
};

void router_info_new(struct router_info ** ri)
{
  (*ri) = mallocx(sizeof(struct router_info), MALLOCX_ZERO);
}

void router_info_free(struct router_info ** ri)
{
  mnet_identity_free(&(*ri)->identity);
  size_t n = (*ri)->num_addresses;
  while(n--)
    mnet_addr_free(&(*ri)->addresses[n]);
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
  int l = ri->len - mnet_identity_siglen(ri->identity);
  if(!l) mnet_error(LOG_DATA, "router info has length of size 0");
  return l > 0 && mnet_identity_verify_data(ri->identity, ri->data, l, ri->data + l);
}

void router_info_generate(struct mnet_identity_keys * k, struct router_info_config * config, struct router_info ** info)
{
  router_info_new(info);
  mnet_identity_keys_to_public(k, &(*info)->identity);
  if(config->caps)
    (*info)->caps = strdup(config->caps);
}

int router_info_write(struct router_info * ri, FILE * f)
{
  if(!ri->len) return 0; // empty
  return fwrite(ri->data, ri->len, 1, f) != ri->len;
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
  mnet_base64_encode(h, sizeof(ident_hash), s, sizeof(s));
  return strdup(s);
}

void router_info_hash(struct router_info * ri, ident_hash * ident)
{
  mnet_identity_hash(ri->identity, ident);
}

void router_info_get_caps(struct router_info * ri, char ** caps)
{
  if(ri->caps) *caps = strdup(ri->caps);
  else *caps = NULL;
}

void router_info_get_identity(struct router_info * ri, struct mnet_identity ** ident)
{
  *ident = ri->identity;
}

void router_info_config_new(struct router_info_config ** c)
{
  *c = xmalloc(sizeof(struct router_info_config));
}

void router_info_config_free(struct router_info_config ** c)
{
  free((*c)->caps);
  free(*c);
  *c = NULL;
}
