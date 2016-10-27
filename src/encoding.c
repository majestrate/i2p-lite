#include <i2pd/encoding.h>

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



size_t                                /* Number of bytes in the encoded buffer */
i2p_base64_encode(uint8_t * InBuffer,           /* Input buffer, binary data */
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



size_t i2p_base32_encode (uint8_t * inBuf, size_t len, char * outBuf, size_t outLen)
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
