#ifndef I2PD_NETDB_H_
#define I2PD_NETDB_H_

#include <i2pd/datatypes.h>
#include <i2pd/ri.h>
#include <i2pd/types.h>

#define I2P_CONFIG_NETDB_DIR "i2p.netdb.dir"

struct i2p_netdb;

struct i2p_netdb_config
{
  i2p_filename rootdir;
};

#define default_netdb_config { "netDb" }

// entry in the netdb
typedef struct
{
  ident_hash ident;
  struct router_info * ri;
} netdb_entry;


void i2p_netdb_new(struct i2p_netdb ** db, struct i2p_netdb_config c);

void i2p_netdb_free(struct i2p_netdb ** db);

/** @brief put a router info into the netdb, updates it if it already is loaded, returns 1 on success otherwise returns 0 */
int i2p_netdb_put_router_info(struct i2p_netdb * db, struct router_info * ri);

/** @brief find a router info by ident hash, loads it if not loaded returns 1 if found otherwise returns 0 if not found */
int i2p_netdb_find_router_info(struct i2p_netdb * db, ident_hash * ident, struct router_info ** ri);

/** @brief flush all entries in ram to disk, returns 1 on success otherwise returns 0 */
int i2p_netdb_flush_to_disk(struct i2p_netdb * db);

/** @brief load all netdb entries on disk into memory */
int i2p_netdb_load_all(struct i2p_netdb * db);

/** @brief netdb iterator type */
typedef void(*netdb_itr)(netdb_entry *, void *);

/** @brief iterate over all entries in netdb */
void i2p_netdb_for_each(struct i2p_netdb * db, netdb_itr i, void * user);

#endif
