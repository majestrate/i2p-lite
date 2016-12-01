#ifndef I2PD_NETDB_H_
#define I2PD_NETDB_H_

#include <i2pd/datatypes.h>
#include <i2pd/ri.h>
#include <i2pd/types.h>
#include <i2pd/i2np.h>

#define I2P_CONFIG_NETDB_DIR "i2p.netdb.dir"

#ifndef NETDB_MIN_PEERS
// minimum number of routers we need for network to work
#define NETDB_MIN_PEERS 20
#endif

struct i2p_netdb;

// entry in the netdb
typedef struct
{
  ident_hash ident;
  struct router_info * ri;
} netdb_entry;


void i2p_netdb_new(struct i2p_netdb ** db, const char * dir);

void i2p_netdb_free(struct i2p_netdb ** db);

/** @brief put a router info into the netdb, updates it if it already is loaded, returns 1 on success otherwise returns 0 */
int i2p_netdb_put_router_info(struct i2p_netdb * db, struct router_info * ri);

/** @brief find a router info by ident hash, loads it if not loaded returns 1 if found otherwise returns 0 if not found */
int i2p_netdb_find_router_info(struct i2p_netdb * db, ident_hash ident, struct router_info ** ri);

/** @brief flush all entries in ram to disk, returns 1 on success otherwise returns 0 */
int i2p_netdb_flush_to_disk(struct i2p_netdb * db);

/** @brief ensure filesystem skiplist structure exists return 1 on successs otherwise return 0 */
int i2p_netdb_ensure_skiplist(struct i2p_netdb * db);

/** @brief load all netdb entries on disk into memory */
int i2p_netdb_load_all(struct i2p_netdb * db);

/** @brief return how many peers we have loaded in memory */
size_t i2p_netdb_loaded_peer_count(struct i2p_netdb * db);

/** @brief netdb iterator type */
typedef void(*netdb_iterator)(ident_hash k, struct router_info * v, void *);

/** @brief iterate over all entries in netdb memory */
void i2p_netdb_for_each(struct i2p_netdb * db, netdb_iterator i, void * user);

struct i2np_message_router * i2p_netdb_message_router(struct i2p_netdb * db);

#endif
