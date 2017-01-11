#ifndef MNET_TRANSPORT_H_
#define MNET_TRANSPORT_H_
#include <mnet/garlic.h>
#include <mnet/datatypes.h>
#include <mnet/netdb.h>

#include <uv.h>

/** @brief transport implementation details */
struct mnet_garlic_transport_impl
{
  void * impl; // implementation detail
  int (*sendto)(void*, struct mnet_garlic_msg *, ident_hash); // queue sending a message to a router by hash
  const char * name; // name of this transport
};

/** @brief transport layer muxer */
struct mnet_garlic_transport;

/** @brief create new transport layer muxer with initialized uv_loop and message router */
void mnet_garlic_transport_new(struct mnet_garlic_transport ** t, uv_loop_t * loop);
void mnet_garlic_transport_free(struct mnet_garlic_transport ** t);

/** @brief register transport impl with transport,  return 1 if we registered successfully otherwise return 0 */
int mnet_garlic_transport_register(struct mnet_garlic_transport * t, struct mnet_garlic_transport_impl * i);
/** @brief deregister transport impl with transport, does nothing if not registered */
void mnet_garlic_transport_deregister(struct mnet_garlic_transport * t, struct mnet_garlic_transport_impl * i);

/** @brief send an i2np message to a router given its ident hash, returns -1 on error, otherwise returns 0 */
int mnet_garlic_sendto(struct mnet_garlic_transport * t, struct mnet_garlic_msg * msg, ident_hash to);

#endif
