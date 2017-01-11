#ifndef I2PD_NETDB_H_
#define I2PD_NETDB_H_

#include <mnet/datatypes.h>
#include <mnet/ri.h>
#include <mnet/types.h>
#include <mnet/garlic.h>

#define MNET_CONFIG_NETDB_DIR "mnet.netdb.dir"

#ifndef NETDB_MIN_PEERS
// minimum number of routers we need for network to work
#define NETDB_MIN_PEERS 20
#endif


struct mnet_netdb;

// entry in the netdb
typedef struct
{
  ident_hash ident;
  struct router_info * ri;
} netdb_entry;


void mnet_netdb_new(struct mnet_netdb ** db, const char * dir);

void mnet_netdb_free(struct mnet_netdb ** db);

/** @brief put a router info into the netdb, updates it if it already is loaded, returns 1 on success otherwise returns 0 */
int mnet_netdb_put_router_info(struct mnet_netdb * db, struct router_info * ri);

/** @brief find a router info by ident hash, loads it if not loaded returns 1 if found otherwise returns 0 if not found */
int mnet_netdb_find_router_info(struct mnet_netdb * db, ident_hash ident, struct router_info ** ri);

/** @brief flush all entries in ram to disk, returns 1 on success otherwise returns 0 */
int mnet_netdb_flush_to_disk(struct mnet_netdb * db);

/** @brief ensure filesystem skiplist structure exists return 1 on successs otherwise return 0 */
int mnet_netdb_ensure_skiplist(struct mnet_netdb * db);

/** @brief load all netdb entries on disk into memory */
int mnet_netdb_load_all(struct mnet_netdb * db);

/** @brief return how many peers we have loaded in memory */
size_t mnet_netdb_loaded_peer_count(struct mnet_netdb * db);

/** @brief netdb iterator type */
typedef void(*netdb_iterator)(ident_hash k, struct router_info * v, void *);

/** @brief iterate over all entries in netdb memory */
void mnet_netdb_for_each(struct mnet_netdb * db, netdb_iterator i, void * user);

struct mnet_garlic_message_router * mnet_netdb_message_router(struct mnet_netdb * db);

#endif
