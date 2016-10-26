#ifndef I2PD_CONFIG_H_
#define I2PD_CONFIG_H_

/** write default config to file */
int i2p_config_gen(char * filepath);

struct i2p_config;

int i2p_config_load(struct i2p_config ** cfg, const char * filepath);
void i2p_config_free(struct i2p_config ** cfg);

typedef void(*i2p_config_iter_t)(char *, char *, void *);

void i2p_config_for_each(struct i2p_config * cfg, i2p_config_iter_t iter, void * user);
#endif
