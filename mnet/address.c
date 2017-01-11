#include <mnet/address.h>
#include <mnet/encoding.h>
#include <mnet/log.h>
#include <mnet/memory.h>
#include <mnet/util.h>

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

void mnet_addr_process_dict(char * k, char * v, void * u)
{
  struct mnet_addr * a = (struct mnet_addr * ) u;  
  mnet_debug(LOG_DATA, "address dict: %s = %s", k, v);
  if(!strcmp(k, "host")) {
    a->host = strdup(v);
    return;
  }
  if(!strcmp(k, "port")) {
    a->port = htons(atoi(v));
    return;
  }
  if(!strcmp(k, "key")) {
    // for ssu
    uint8_t * key = NULL;
    size_t len = mnet_base64_decode_str(v, &key);
    if (len == sizeof(pub_enc_key_t)) {
      memcpy(a->key, key, len);
    }
    free(key);
  }
}

uint8_t * mnet_addr_read_dict(struct mnet_addr ** addr, uint8_t * b, size_t l)
{
  return NULL;
}

void mnet_addr_free(struct mnet_addr ** addr)
{
  free((*addr)->style);
  free(*addr);
  *addr = NULL;
}

char * mnet_addr_port_str(struct mnet_addr * addr)
{
  char buf[10] = {0};
  snprintf(buf, sizeof(buf), "%d", ntohs(addr->port));
  return strdup(buf);
}
