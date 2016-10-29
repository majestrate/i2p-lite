#include <i2pd/transport.h>
#include <i2pd/memory.h>

struct i2np_transport
{
  uv_loop_t * loop;
};


void i2np_transport_new(struct i2np_transport ** t, uv_loop_t * l)
{
  (*t) = mallocx(sizeof(struct i2np_transport), MALLOCX_ZERO);
  (*t)->loop = l;
}

void i2np_transport_free(struct i2np_transport ** t)
{
}

uv_loop_t * i2np_transport_get_loop(struct i2np_transport * t)
{
  return t->loop;
}
