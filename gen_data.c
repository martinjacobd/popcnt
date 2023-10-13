#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint64_t rol64(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

struct xoshiro256ss_state {
  uint64_t s[4];
};

uint64_t xoshiro256ss(struct xoshiro256ss_state *state)
{
  uint64_t *s = state->s;
  uint64_t const result = rol64(s[1] * 5, 7) * 9;
  uint64_t const t = s[1] << 17;
  
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  
  s[2] ^= t;
  s[3] = rol64(s[3], 45);
  
  return result;
}

#define LEAFLABS 0x1eaf1ab51eaf1ab5
/* 0.5 GB */
#define N_QWORDS (1 << 26)

int main(void) {
  struct xoshiro256ss_state state = {
    .s = { LEAFLABS, LEAFLABS, LEAFLABS, LEAFLABS },
  };
  FILE *f = fopen("data.bin", "w+");

  for (size_t i = 0; i < N_QWORDS; i++) {
    uint64_t r = xoshiro256ss(&state);
    fwrite(&r, sizeof(r), 1, f);
  }

  return 0;
}
