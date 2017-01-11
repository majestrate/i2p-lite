#include <i2pd/cert.h>
#include <i2pd/memory.h>
#include <i2pd/log.h>

struct i2p_cert
{
  uint8_t * data;
  size_t len;
};

void i2p_cert_new(struct i2p_cert ** c)
{
  *c = xmalloc(sizeof(struct i2p_cert));
}

void i2p_cert_init(struct i2p_cert * c, uint8_t type, uint8_t * data, uint16_t len)
{
  c->len = len + 3;
  c->data = xmalloc(c->len);
  c->data[0] = type;
  htobe16buf(c->data+1, len);
  if (len) memcpy(c->data+3, data, len);
}

int i2p_cert_read(struct i2p_cert * c, FILE * f)
{
  uint8_t b[3];
  if(fread(b, sizeof(uint8_t), 3, f) != 3) {
    // read error
    return 0;
  }
  c->len = bufbe16toh(b+1) + 3;
  c->data = xmalloc(c->len);
  memcpy(c->data, b, 3);
  if(c->len) {
    int l = c->len - 3;
    if(fread(c->data + 3, l, 1, f) != l) {
      // read error
      return 0;
    }
  }
  return 1;
}

int i2p_cert_write(struct i2p_cert * c, FILE * f)
{
  return fwrite(c->data, c->len, 1, f) == c->len;
}

uint8_t * i2p_cert_read_buffer(struct i2p_cert * c, uint8_t * d, size_t len)
{
  if(len < 3) {
    // underflow
    return NULL;
  }
  c->len = bufbe16toh(d+1) + 3;
  c->data = xmalloc(c->len);
  c->data[0] = *d;
  
  if(c->len) {
    if((c->len - 3) > len) { // overflow
      i2p_error(LOG_DATA, "i2p certificate overflow: %lu > %lu", c->len - 3, len);
      return NULL;
    }
    memcpy(c->data + 3, d + 3, c->len - 3);
  }
  return d + c->len;
}

void i2p_cert_free(struct i2p_cert ** c)
{
  free((*c)->data);
  free(*c);
  *c = NULL;
}

uint8_t * i2p_cert_buffer(struct i2p_cert * c)
{
  return c->data;
}

uint16_t i2p_cert_buffer_length(struct i2p_cert * c)
{
  return c->len;
}

uint8_t * i2p_cert_data(struct i2p_cert * c)
{
  return c->data + 3;
}

uint16_t i2p_cert_data_length(struct i2p_cert * c)
{
  return c->len - 3;
}

uint8_t i2p_cert_type(struct i2p_cert * c)
{
  return c->data[0];
}

