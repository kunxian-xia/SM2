#ifdef __cplusplus
extern "C" {
#endif

#include "sm3.h"
#include <limits.h>
#include <string.h>

#define byte_swap16 __builtin_bswap16
#define byte_swap32 __builtin_bswap32
#define byte_swap64 __builtin_bswap64

#define to_bigendian16 byte_swap16
#define to_bigendian32 byte_swap32
#define to_bigendian64 byte_swap64

#define bitsof(x) (sizeof(x) * CHAR_BIT)
#define rol(x, n) (((x) << (n)) | ((x) >> ((bitsof(x) - (n)))))
#define ror(x, n) (((x) >> (n)) | ((x) << ((bitsof(x) - (n)))))

void sm3_init(sm3_ctx_t *ctx) {
  ctx->digest[0] = 0x7380166F;
  ctx->digest[1] = 0x4914B2B9;
  ctx->digest[2] = 0x172442D7;
  ctx->digest[3] = 0xDA8A0600;
  ctx->digest[4] = 0xA96F30BC;
  ctx->digest[5] = 0x163138AA;
  ctx->digest[6] = 0xE38DEE4D;
  ctx->digest[7] = 0xB0FB0E4E;

  ctx->nblocks = 0;
  ctx->num = 0;
}

static void sm3_compress(unsigned int digest[32 / sizeof(unsigned int)],
                         const unsigned char block[64]);

void sm3_update(sm3_ctx_t *ctx, const unsigned char *data, size_t data_len) {
  if (ctx->num) {
    unsigned int left = 64 - ctx->num;
    if (data_len < left) {
      memcpy(ctx->block + ctx->num, data, data_len);
      ctx->num += data_len;
      return;
    } else {
      memcpy(ctx->block + ctx->num, data, left);
      sm3_compress(ctx->digest, ctx->block);
      ctx->nblocks++;
      data += left;
      data_len -= left;
    }
  }
  while (data_len >= 64) {
    sm3_compress(ctx->digest, data);
    ctx->nblocks++;
    data += 64;
    data_len -= 64;
  }
  ctx->num = data_len;
  if (data_len) { memcpy(ctx->block, data, data_len); }
}

void sm3_final(sm3_ctx_t *ctx, unsigned char *digest) {
  size_t i;
  unsigned int *pdigest = (unsigned int *)(digest);
  uint64_t *count = (uint64_t *)(ctx->block + 64 - 8);

  ctx->block[ctx->num] = 0x80;

  if (ctx->num + 9 <= 64) {
    memset(ctx->block + ctx->num + 1, 0, 64 - ctx->num - 9);
  } else {
    memset(ctx->block + ctx->num + 1, 0, 64 - ctx->num - 1);
    sm3_compress(ctx->digest, ctx->block);
    memset(ctx->block, 0, 64 - 8);
  }

  count[0] = ((uint64_t)(ctx->nblocks) << 9) + (ctx->num << 3);
  count[0] = to_bigendian64(count[0]);

  sm3_compress(ctx->digest, ctx->block);
  for (i = 0; i < sizeof(ctx->digest) / sizeof(ctx->digest[0]); i++) {
    pdigest[i] = to_bigendian32(ctx->digest[i]);
  }
}

void sm3_hash(const unsigned char *msg, size_t msg_len,
              unsigned char dgst[32]) {
  sm3_ctx_t ctx;

  sm3_init(&ctx);
  sm3_update(&ctx, msg, msg_len);
  sm3_final(&ctx, dgst);

  memset(&ctx, 0, sizeof(sm3_ctx_t));
}

static int mem_compare(const unsigned char *b1_, const unsigned char *b2_, size_t len) {
  const volatile unsigned char *volatile b1 = (const volatile unsigned char *volatile)b1_;
  const volatile unsigned char *volatile b2 = (const volatile unsigned char *volatile)b2_;
  unsigned char gt = 0U;
  unsigned char eq = 1U;
  size_t i;

  i = len;
  while (i != 0U) {
    i--;
    gt |= ((b2[i] - b1[i]) >> 8) & eq;
    eq &= ((b2[i] ^ b1[i]) - 1) >> 8;
  }
  return (int)(gt + gt + eq) - 1;
}

int sm3_hash_verify(const unsigned char *data, size_t data_len, const unsigned char *digest,
                    size_t digest_len) {
  if (digest_len > 32) return -1;

  unsigned char hash_value[32];
  sm3_hash(data, data_len, hash_value);

  return mem_compare(digest, hash_value, digest_len);
}

#define P0(x) ((x) ^ rol((x), 9) ^ rol((x), 17))
#define P1(x) ((x) ^ rol((x), 15) ^ rol((x), 23))

#define FF0(x, y, z) ((x) ^ (y) ^ (z))
#define FF1(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))

#define GG0(x, y, z) ((x) ^ (y) ^ (z))
#define GG1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))

static void sm3_compress(unsigned int digest[32 / sizeof(unsigned int)],
                         const unsigned char block[64]) {
  int j;
  unsigned int W[68], W1[64];
  const unsigned int *pblock = (const unsigned int *)(block);

  unsigned int A = digest[0], B = digest[1], C = digest[2], D = digest[3];
  unsigned int E = digest[4], F = digest[5], G = digest[6], H = digest[7];

  unsigned int SS1, SS2, TT1, TT2, T[64];

  for (j = 0; j < 16; j++) { W[j] = to_bigendian32(pblock[j]); }
  for (j = 16; j < 68; j++) {
    W[j] = P1(W[j - 16] ^ W[j - 9] ^ rol(W[j - 3], 15)) ^ rol(W[j - 13], 7) ^
        W[j - 6];
  }
  for (j = 0; j < 64; j++) { W1[j] = W[j] ^ W[j + 4]; }

  for (j = 0; j < 16; j++) {
    T[j] = 0x79CC4519;
    SS1 = rol((rol(A, 12) + E + rol(T[j], j)), 7);
    SS2 = SS1 ^ rol(A, 12);
    TT1 = FF0(A, B, C) + D + SS2 + W1[j];
    TT2 = GG0(E, F, G) + H + SS1 + W[j];
    D = C;
    C = rol(B, 9);
    B = A;
    A = TT1;
    H = G;
    G = rol(F, 19);
    F = E;
    E = P0(TT2);
  }

  for (j = 16; j < 64; j++) {
    T[j] = 0x7A879D8A;
    SS1 = rol((rol(A, 12) + E + rol(T[j], j)), 7);
    SS2 = SS1 ^ rol(A, 12);
    TT1 = FF1(A, B, C) + D + SS2 + W1[j];
    TT2 = GG1(E, F, G) + H + SS1 + W[j];
    D = C;
    C = rol(B, 9);
    B = A;
    A = TT1;
    H = G;
    G = rol(F, 19);
    F = E;
    E = P0(TT2);
  }

  digest[0] ^= A, digest[1] ^= B, digest[2] ^= C, digest[3] ^= D;
  digest[4] ^= E, digest[5] ^= F, digest[6] ^= G, digest[7] ^= H;
}

#ifdef __cplusplus
}
#endif
