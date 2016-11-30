#include <i2pd/address.h>
#include <i2pd/log.h>
#include <i2pd/memory.h>
#include <i2pd/util.h>

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

void i2p_addr_process_dict(char * k, char * v, void * u)
{
  struct i2p_addr * a = (struct i2p_addr * ) u;  
  i2p_debug(LOG_DATA, "i2p address dict: %s = %s", k, v);
  if(!strcmp(k, "host")) {
    a->host = strdup(v);
    return;
  }
  if(!strcmp(k, "port")) {
    a->port = htons(atoi(v));
  }
}

uint8_t * i2p_addr_read_dict(struct i2p_addr ** addr, uint8_t * b, size_t l)
{
  (*addr) = mallocx(sizeof(struct i2p_addr), MALLOCX_ZERO);
  (*addr)->cost = *b;
  b ++ ;
  (*addr)->date = bufbe64toh(b);
  b += sizeof(uint64_t);
  uint8_t * d = read_i2pstring(b, l, &(*addr)->style);
  i2p_debug(LOG_DATA, "address style %s", (*addr)->style);
  return read_i2pdict(d, l + 1 + strlen((*addr)->style), i2p_addr_process_dict, (*addr));
}

void i2p_addr_free(struct i2p_addr ** addr)
{
  free((*addr)->style);
  free(*addr);
  *addr = NULL;
}

char * i2p_addr_port_str(struct i2p_addr * addr)
{
  char buf[10] = {0};
  snprintf(buf, sizeof(buf), "%d", ntohs(addr->port));
  return strdup(buf);
}
