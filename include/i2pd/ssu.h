#ifndef I2PD_SSU_H_
#define I2PD_SSU_H_

struct ssu_config
{
  // address to bind to
  i2p_hostname addr;
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

#endif
