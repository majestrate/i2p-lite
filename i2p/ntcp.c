#include <i2pd/ntcp.h>
#include <i2pd/memory.h>

#include <assert.h>
#include <uv.h>

struct ntcp_server
{
  uv_tcp_t * serv;
};

struct ntcp_conn
{
  uv_tcp_t * conn;
  uint8_t readbuff[NTCP_BUFF_SIZE];
};

void ntcp_server_alloc(struct ntcp_server ** s)
{
  (*s) = mallocx(sizeof(struct ntcp_server), MALLOCX_ZERO);
  (*s)->serv = mallocx(sizeof(uv_tcp_t), MALLOCX_ZERO);
}

void ntcp_server_free(struct ntcp_server ** s)
{
  free((*s)->serv);
  free(*s);
  *s = NULL;
}

void ntcp_server_configure(struct ntcp_server * s, struct ntcp_config c)
{
  // associate user data with handle
  s->serv->data = s;
}

void ntcp_server_attach(struct ntcp_server * s, struct i2np_transport * t)
{
  uv_loop_t * loop = i2np_transport_get_loop(t);
  assert(uv_tcp_init(loop, s->serv) != -1);
}

// called when ntcp server closes socket
void ntcp_server_closed_callback(uv_handle_t * h)
{

}

void ntcp_server_detach(struct ntcp_server * s)
{
  uv_close((uv_handle_t*)s->serv, ntcp_server_closed_callback);
}
