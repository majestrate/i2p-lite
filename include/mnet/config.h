#ifndef MNET_CONFIG_H_
#define MNET_CONFIG_H_

/** write default config to file */
int mnet_config_gen(char * filepath);

struct mnet_config;

int mnet_config_load(struct mnet_config ** cfg, const char * filepath);
void mnet_config_free(struct mnet_config ** cfg);

typedef void(*mnet_config_iter_t)(char *, char *, void *);

void mnet_config_for_each(struct mnet_config * cfg, mnet_config_iter_t iter, void * user);
#endif
