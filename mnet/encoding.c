#include <mnet/encoding.h>
#include <mnet/memory.h>
#include <stdlib.h>

// implementation lifted from i2pd

size_t mnet_base64_encoding_buffer_size (size_t input_size) 
{
  div_t d = div (input_size, 3);
  if (d.rem) d.quot++;
  return 4*d.quot;
}

char * mnet_base64_encode_str(uint8_t * buf, size_t len)
{
  size_t outlen = mnet_base64_encoding_buffer_size(len);
  char * str = xmalloc(outlen+1);
  mnet_base64_encode(buf, len, str, outlen);
  return str;
}

size_t mnet_base64_decode_str(char * in, uint8_t ** out)
{
  size_t inlen = strlen(in);
  // TODO: buffer is uneedingly too big
  uint8_t * temp = xmalloc(inlen);
  size_t outlen = mnet_base64_decode(in, inlen, temp, inlen);
  if (outlen) {
    *out = temp;
  } else {
    free(temp);
    *out = NULL;
  }
  return outlen;
}

static const char T32[32] = {
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
  'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
  'y', 'z', '2', '3', '4', '5', '6', '7',
};


static const char T64[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '-', '~'
};

static char P64 = '='; 

const char * MNET_BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-~";

/*
 * Reverse Substitution Table (built in run time)
 */

static char iT64[256];
int isFirstTime = 1;

static void iT64Build()
{
  int  i;
  isFirstTime = 0;
  for ( i=0; i<256; i++ ) iT64[i] = -1;
  for ( i=0; i<64; i++ ) iT64[(int)T64[i]] = i;
  iT64[(int)P64] = 0;
}

size_t                              /* Number of output bytes */
mnet_base64_decode ( 
  char * InBuffer,           /* BASE64 encoded buffer */
  size_t    InCount,          /* Number of input bytes */
  uint8_t  * OutBuffer,	/* output buffer length */ 	
  size_t len)         	/* length of output buffer */
	{
		unsigned char * ps;
		unsigned char * pd;
		unsigned char   acc_1;
		unsigned char   acc_2;
		int             i; 
		int             n; 
		int             m; 
		size_t outCount;

		if (isFirstTime) iT64Build();
		n = InCount/4;
		m = InCount%4;
		if (InCount && !m) 
		     outCount = 3*n;
		else {
		     outCount = 0;
		     return 0;
		}
		
		ps = (unsigned char *)(InBuffer + InCount - 1);
		while ( *ps-- == P64 ) outCount--;
		ps = (unsigned char *)InBuffer;
		
		if (outCount > len) return -1;
		pd = OutBuffer;
		uint8_t * endOfOutBuffer = OutBuffer + outCount;		
		for ( i = 0; i < n; i++ ){
		     acc_1 = iT64[*ps++];
		     acc_2 = iT64[*ps++];
		     acc_1 <<= 2;
		     acc_1 |= acc_2>>4;
		     *pd++  = acc_1;
			 if (pd >= endOfOutBuffer) break;

		     acc_2 <<= 4;
		     acc_1 = iT64[*ps++];
		     acc_2 |= acc_1 >> 2;
		     *pd++ = acc_2;
			  if (pd >= endOfOutBuffer) break;	

		     acc_2 = iT64[*ps++];
		     acc_2 |= acc_1 << 6;
		     *pd++ = acc_2;
		}

		return outCount;
	}


size_t                                /* Number of bytes in the encoded buffer */
mnet_base64_encode(uint8_t * InBuffer,           /* Input buffer, binary data */
                  size_t    InCount,              /* Number of bytes in the input buffer */ 
                  char  * OutBuffer,          /* output buffer */
                  size_t len			   /* length of output buffer */	             
                  )
  
{
  unsigned char * ps;
  unsigned char * pd;
  unsigned char   acc_1;
  unsigned char   acc_2;
  int             i; 
  int             n; 
  int             m; 
  size_t outCount;

  ps = (unsigned char *)InBuffer;
  n = InCount/3;
  m = InCount%3;
  if (!m)
    outCount = 4*n;
  else
    outCount = 4*(n+1);
  if (outCount > len) return 0;
  pd = (unsigned char *)OutBuffer;
  for ( i = 0; i<n; i++ ) {
    acc_1 = *ps++;
    acc_2 = (acc_1<<4)&0x30; 
    acc_1 >>= 2;              /* base64 digit #1 */
    *pd++ = T64[acc_1];
    acc_1 = *ps++;
    acc_2 |= acc_1 >> 4;      /* base64 digit #2 */
    *pd++ = T64[acc_2];
    acc_1 &= 0x0f;
    acc_1 <<=2;
    acc_2 = *ps++;
    acc_1 |= acc_2>>6;        /* base64 digit #3 */
    *pd++ = T64[acc_1];
    acc_2 &= 0x3f;            /* base64 digit #4 */
    *pd++ = T64[acc_2];
  } 
  if ( m == 1 ) {
    acc_1 = *ps++;
    acc_2 = (acc_1<<4)&0x3f;  /* base64 digit #2 */
    acc_1 >>= 2;              /* base64 digit #1 */
    *pd++ = T64[acc_1];
    *pd++ = T64[acc_2];
    *pd++ = P64;
    *pd++ = P64;

  }
  else if ( m == 2 ) {
    acc_1 = *ps++;
    acc_2 = (acc_1<<4)&0x3f; 
    acc_1 >>= 2;              /* base64 digit #1 */
    *pd++ = T64[acc_1];
    acc_1 = *ps++;
    acc_2 |= acc_1 >> 4;      /* base64 digit #2 */
    *pd++ = T64[acc_2];
    acc_1 &= 0x0f;
    acc_1 <<=2;               /* base64 digit #3 */
    *pd++ = T64[acc_1];
    *pd++ = P64;
  }
  return outCount;
}



size_t mnet_base32_encode (uint8_t * inBuf, size_t len, char * outBuf, size_t outLen)
{
  size_t ret = 0, pos = 1;
  int bits = 8, tmp = inBuf[0];
  while (ret < outLen && (bits > 0 || pos < len)) { 	
    if (bits < 5) {
      if (pos < len) {
        tmp <<= 8;
        tmp |= inBuf[pos] & 0xFF;
        pos++;
        bits += 8;
      }
      else {
        tmp <<= (5 - bits);
        bits = 5;
      }
    }	
		
    bits -= 5;
    int ind = (tmp >> bits) & 0x1F;
    outBuf[ret] = (ind < 26) ? (ind + 'a') : ((ind - 26) + '2');
    ret++;
  }
  return ret;
}
