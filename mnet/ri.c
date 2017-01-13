#include <mnet/ri.h>
#include <mnet/address.h>
#include <mnet/bencode.h>
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
  struct iwp_config * link;
  
  uint8_t * caps;
};

void router_info_new(struct router_info ** ri)
{
  (*ri) = xmalloc(sizeof(struct router_info));
}

void router_info_free(struct router_info ** ri)
{
  mnet_identity_free(&(*ri)->identity);
  iwp_config_free(&(*ri)->link);
  free((*ri)->data);
  free((*ri)->caps);
  free(*ri);
}

typedef struct {
  struct router_info * ri;
  int success;
} ri_read_ctx;

void router_info_process(bencode_obj_t d, const char * k, bencode_obj_t v, void * u)
{
  ri_read_ctx * ctx = (ri_read_ctx *) u;
  struct router_info * ri = ctx->ri;
  if(!strcmp(k, "caps")) {
    // free existing caps
    if(ri->caps) {
      free(ri->caps);
      ri->caps = NULL;
    }
    // read caps
    ssize_t sz = bencode_obj_getstr(v, &ri->caps);
    if (sz == -1) {
      mnet_error(LOG_DATA, "failed to read router info, bad caps");
    } else {
      ri->caps[sz] = 0;
      mnet_debug(LOG_DATA, "read caps from router info: %s", ri->caps);
    }
  }
  else if(!strcmp(k, "iwp")) {
    // iwp config
    if(ri->link) iwp_config_free(&ri->link);
    iwp_config_new(&ri->link);
    if(!iwp_config_load_dict(ri->link, v)) {
      // error
      mnet_error(LOG_DATA, "Failed to parse iwp config");
      iwp_config_free(&ri->link);
    }
  } else if (!strcmp(k, "ident")) {
    if(ri->identity) mnet_identity_free(&ri->identity);
    mnet_identity_new(&ri->identity);
    uint8_t * str = NULL;
    ssize_t sz = bencode_obj_getstr(v, &str);
    if(str) {
      str[sz] = 0;
      if(!mnet_identity_from_base64(ri->identity, str)) {
        mnet_error(LOG_DATA, "failed to parse ident");
      }
    }
  }
}

int router_info_load(struct router_info * ri, FILE * f)
{
  ri_read_ctx ctx = {
    .ri = ri,
    .success = 0
  };
  bencode_obj_t benc = NULL;
  if(bencode_read_file(&benc, f) > 0) { 
    if(bencode_obj_is_dict(benc)) {
      ctx.success = 1;
      bencode_obj_iter_dict(benc, router_info_process, &ctx);
    }
  }
  if (benc) bencode_obj_free(&benc);
  return ctx.success;
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
  int ret = 0;
  bencode_obj_t benc = NULL;
  bencode_obj_t k = NULL;
  char * str = mnet_identity_to_base64(ri->identity);
  bencode_obj_dict(&benc);
  bencode_obj_str(&k, str, strlen(str));
  free(str);
  bencode_obj_dict_set(benc, "ident", k);
  bencode_obj_free(&k);

  bencode_obj_free(&benc);
  return ret;
}

void router_info_iter_addrs(struct router_info * ri, router_info_addr_iter i, void * u)
{
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
