#include <mnet/memory.h>

uint16_t buf16toh(const void *buf)
{
	uint16_t b16;
	memcpy(&b16, buf, sizeof(uint16_t));
	return b16;
}
 
uint32_t buf32toh(const void *buf)
{
	uint32_t b32;
	memcpy(&b32, buf, sizeof(uint32_t));
	return b32;
}

uint64_t buf64toh(const void *buf)
{
	uint64_t b64;
	memcpy(&b64, buf, sizeof(uint64_t));
	return b64;
}

uint16_t bufbe16toh(const void *buf)
{
	return be16toh(buf16toh(buf));
}

uint32_t bufbe32toh(const void *buf)
{
	return be32toh(buf32toh(buf));
}

uint64_t bufbe64toh(const void *buf)
{
	return be64toh(buf64toh(buf));
}

void htobuf16(void *buf, uint16_t b16)
{
	memcpy(buf, &b16, sizeof(uint16_t));
}

void htobuf32(void *buf, uint32_t b32)
{
	memcpy(buf, &b32, sizeof(uint32_t));
}

void htobuf64(void *buf, uint64_t b64)
{
	memcpy(buf, &b64, sizeof(uint64_t));
}

void htobe16buf(void *buf, uint16_t big16)
{
	htobuf16(buf, htobe16(big16));
}

void htobe32buf(void *buf, uint32_t big32)
{
	htobuf32(buf, htobe32(big32));
}

void htobe64buf(void *buf, uint64_t big64)
{
	htobuf64(buf, htobe64(big64));
}
