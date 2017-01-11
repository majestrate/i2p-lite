#ifndef I2PD_RI_H_
#define I2PD_RI_H_
#include <i2pd/datatypes.h>
#include <i2pd/address.h>
#include <i2pd/identity.h>
#include <i2pd/i2np.h>
#include <i2pd/elg.h>

#include <stdint.h>

/** @brief parameters for initializing a router info */
struct router_info_config
{
  /** iwp address info */
  struct iwp_config * iwp;
  /** ntcp address info */
  struct ntcp_config * ntcp;
  /** ssu address info */
  struct ssu_config * ssu;
  /** router caps, i.e. ORfX */
  char * caps;
  /** set to 1 if we should publish to netdb, otherwise set to 0 in which we won't publish to the network */
  int publish;
};

void router_info_config_new(struct router_info_config ** cfg);
void router_info_config_free(struct router_info_config ** cfg);


struct router_info;

void router_info_new(struct router_info ** ri);
void router_info_free(struct router_info ** ri);

/** @brief load router info from open file and verify, return 1 on success otherwise return 0 */
int router_info_load(struct router_info * ri, FILE * f);

/** @brief verify router info signature */
int router_info_verify(struct router_info * ri);

/** @brief generate a new router info and sign it */
void router_info_generate(struct i2p_identity_keys * k, struct router_info_config * cfg, struct router_info ** ri);

/** @brief get this router info's caps section, if router info has no caps, caps is set to NULL, caller must free result when done */
void router_info_get_caps(struct router_info * ri, char ** caps);

/** @brief read router info from buffer, return NULL on overflow otherwise return address in buffer were we stopped reading */
uint8_t * router_info_load_buffer(struct router_info * ri, uint8_t * buf, size_t len);

/** @brief calculate ident hash of router info */
void router_info_hash(struct router_info * ri, ident_hash * ident);

/** @brief write router info to file */
int router_info_write(struct router_info * ri, FILE * f);

/** @brief get base64 ident hash of this router info, caller must free returned string */
char * router_info_base64_ident(struct router_info * ri);

/** @brief callback for iterating over all of a router info's addresses */
typedef void (*router_info_addr_iter)(struct router_info *, struct i2p_addr *, void *);

/** @brief iterate over all this router info's provided addresses */
void router_info_iter_addrs(struct router_info * ri, router_info_addr_iter i, void * u);

/** @brief convert this router info into a database store message */
void router_info_to_dbsm(struct router_info * ri, struct dbs_msg ** msg);

void router_info_get_identity(struct router_info * ri, struct i2p_identity ** ident);

#endif
