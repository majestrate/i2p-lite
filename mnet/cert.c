#include <mnet/cert.h>
#include <mnet/memory.h>
#include <mnet/log.h>

struct mnet_cert
{
  uint8_t * data;
  size_t len;
};

void mnet_cert_new(struct mnet_cert ** c)
{
  *c = xmalloc(sizeof(struct mnet_cert));
}

void mnet_cert_init(struct mnet_cert * c, uint8_t type, uint8_t * data, uint16_t len)
{
  c->len = len + 3;
  c->data = xmalloc(c->len);
  c->data[0] = type;
  htobe16buf(c->data+1, len);
  if (len) memcpy(c->data+3, data, len);
}

int mnet_cert_read(struct mnet_cert * c, FILE * f)
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

int mnet_cert_write(struct mnet_cert * c, FILE * f)
{
  return fwrite(c->data, c->len, 1, f) == c->len;
}

uint8_t * mnet_cert_read_buffer(struct mnet_cert * c, uint8_t * d, size_t len)
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
      mnet_error(LOG_DATA, "i2p certificate overflow: %lu > %lu", c->len - 3, len);
      return NULL;
    }
    memcpy(c->data + 3, d + 3, c->len - 3);
  }
  return d + c->len;
}

void mnet_cert_free(struct mnet_cert ** c)
{
  free((*c)->data);
  free(*c);
  *c = NULL;
}

uint8_t * mnet_cert_buffer(struct mnet_cert * c)
{
  return c->data;
}

uint16_t mnet_cert_buffer_length(struct mnet_cert * c)
{
  return c->len;
}

uint8_t * mnet_cert_data(struct mnet_cert * c)
{
  return c->data + 3;
}

uint16_t mnet_cert_data_length(struct mnet_cert * c)
{
  return c->len - 3;
}

uint8_t mnet_cert_type(struct mnet_cert * c)
{
  return c->data[0];
}

