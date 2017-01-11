#ifndef I2PD_SSU_H_
#define I2PD_SSU_H_
#include <i2pd/address.h>
#include <stdint.h>

struct ssu_config
{
  // address to bind to
  char * addr;
  // port to bind to
  uint16_t port;
  // try binding to v4?
  int try_ip4;
  // try binding to v6?
  int try_ip6;
  // ipv4 mtu
  int mtu4;
  // ipv6 mtu
  int mtu6;
};

#define default_ssu_config { NULL, 1234, 1, 1, -1, -1 }

void ssu_config_new(struct ssu_config ** conf);
void ssu_config_free(struct ssu_config ** conf);


void ssu_config_to_address(struct ssu_config * c, struct i2p_addr **  addr);

#endif
