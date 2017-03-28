#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

// need 32 bytes to store the 256-bit output hash value of SM3

typedef struct {
  unsigned int digest[32 / sizeof(unsigned int)];
  int nblocks;
  unsigned char block[64];
  int num;
} sm3_ctx_t;

// suitable for dealing with stream data, where you don't know the total length
// of a message.
void sm3_init(sm3_ctx_t *ctx);
void sm3_update(sm3_ctx_t *ctx, const unsigned char *data, size_t data_len);
void sm3_final(sm3_ctx_t *ctx, unsigned char digest[32]);

// compute the hash value of @data and store the hash value into @digest
// sm3_hash() is just a wrapper of the above three functions.
void sm3_hash(const unsigned char *data, size_t data_len,
              unsigned char digest[32]);

// check if the provided @digest is actually the hash value of @data
// return zero for true; otherwise, return nonzero
int sm3_hash_verify(const unsigned char *data, size_t data_len, const unsigned char *digest,
                    size_t digest_len);

#ifdef __cplusplus
}
#endif
