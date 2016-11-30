#include "transport_internal.h"
#include <i2pd/memory.h>


void i2np_transport_new(struct i2np_transport ** t, uv_loop_t * l)
{
  (*t) = mallocx(sizeof(struct i2np_transport), MALLOCX_ZERO);
  (*t)->loop = l;
}

void i2np_transport_free(struct i2np_transport ** t)
{
  free(*t);
  *t = NULL;
}
