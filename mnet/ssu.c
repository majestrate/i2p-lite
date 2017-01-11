#include <i2pd/ssu.h>
#include <i2pd/memory.h>

void ssu_config_new(struct ssu_config ** conf)
{
  *conf = xmalloc(sizeof(struct ssu_config));
}

void ssu_config_free(struct ssu_config ** conf)
{
  free((*conf)->addr);
  free(*conf);
}

void ssu_config_to_address(struct ssu_config * conf, struct i2p_addr ** addr)
{
  *addr = xmalloc(sizeof(struct i2p_addr));
  (*addr)->style = strdup("SSU");
  (*addr)->host = strdup(conf->addr);
  (*addr)->port = conf->port;
}
