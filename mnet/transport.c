#include "transport_internal.h"
#include <mnet/memory.h>
#include <string.h>


void mnet_garlic_transport_new(struct mnet_garlic_transport ** t, uv_loop_t * l)
{
  (*t) = xmalloc(sizeof(struct mnet_garlic_transport));
  (*t)->loop = l;
}

void mnet_garlic_transport_free(struct mnet_garlic_transport ** t)
{
  free(*t);
  *t = NULL;
}

/** return 1 if we have already registered a transport give its name otherwise return 0 */
static int mnet_garlic_transport_impl_is_registered(struct mnet_garlic_transport * t, const char * name)
{
  size_t idx = 0;
  while(idx < MAX_GARLIC_TRANSPORTS) {
    if(t->links[idx].name && ! strcmp(t->links[idx].name, name)) {
      // found
      return 1;
    }
    idx ++;
  }
  return 0;
}

int mnet_garlic_transport_register(struct mnet_garlic_transport * t, struct mnet_garlic_transport_impl * i)
{
  if(mnet_garlic_transport_impl_is_registered(t, i->name)) return 0; // already registered a transport with this name
  size_t idx = 0;
  while(idx < MAX_GARLIC_TRANSPORTS) {
    if(!t->links[idx].name) {
      // found empty slot
      t->links[idx].name = i->name;
      t->links[idx].sendto = i->sendto;
      t->links[idx].impl = i->impl;
      return 1;
    }
    idx ++;
  }
  return 0;
}

void mnet_garlic_transport_deregister(struct mnet_garlic_transport * t, struct mnet_garlic_transport_impl * i)
{
  size_t idx = 0;
  while (idx < MAX_GARLIC_TRANSPORTS) {
    if(t->links[idx].name && !strcmp(i->name, t->links[idx].name)) {
      // found
      t->links[idx].name = NULL;
      t->links[idx].sendto = NULL;
      t->links[idx].impl = NULL;
      return;
    }
    idx ++;
  }
}
