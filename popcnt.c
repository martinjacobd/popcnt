#include <stdint.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#define T uint64_t
#define Return_T uint32_t // can hold the popcnt of up to 2**26 of T
#define BIT_WIDTH (sizeof(T) * CHAR_BIT)
#define bit_counter unsigned char

static const unsigned char byte_bit_table[] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

Return_T popcnt_naive(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n = arry[i];
    for (bit_counter j = 0; j < BIT_WIDTH; j++) {
      ret += n & 0x01;
      n >>= 1;
    }
  }
  return ret;
}

#define DO32(stmt)			\
  {					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  					\
  stmt;					\
  stmt;					\
  }

Return_T popcnt_naive_unrolled(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n = arry[i];
    DO32(ret += n & 0x01L; n >>= 1);
    DO32(ret += n & 0x01L; n >>= 1);
  }
  return ret;
}

Return_T popcnt_naive_improved(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n = arry[i];
    while (n != 0) {
      ret += n & 0x01;
      n >>= 1;
    }
  }
  return ret;
}

Return_T popcnt_naive_improved_branching(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n = arry[i];
    while (n != 0) {
      if (n & 0x01) {
        ret++;
      }
      n >>= 1;
    }
  }
  return ret;
}

Return_T popcnt_kernighan(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n = arry[i];
    while (n) {
      ret++;
      n &= n - 1; /* clear lowest high bit */
    }
  }
  return ret;
}

Return_T popcnt_builtin(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    ret += __builtin_popcountll(arry[i]);
  }
  return ret;
}

Return_T popcnt_inline_asm(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T local_popcnt;
    asm volatile ("popcnt %1, %0"
                  : "=r" (local_popcnt)
                  : "m" (arry[i]));
    ret += local_popcnt;
  }
  return ret;
}

Return_T popcnt_lookup(T arry[], size_t n_elts) {
  Return_T ret = 0;
  uint8_t *arry_reinterpret = (uint8_t *) arry;
  for (size_t i = 0; i < (n_elts * (sizeof(T) / sizeof(uint8_t))); i++) {
    ret += byte_bit_table[arry_reinterpret[i]];
  }
  return ret;
}

Return_T popcnt_magic_nums(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T popcnt_local = 0;
    /* popcnt_local = arry[i] - ((arry[i] >> 1) & (T)~(T)0/3); */
    /* popcnt_local = (popcnt_local & (T)~(T)0/15*3) + ((popcnt_local >> 2) & (T)~(T)0/15*3); */
    /* popcnt_local = (popcnt_local + (popcnt_local >> 4)) & (T)~(T)0/255*15; */
    /* ret += (T)(popcnt_local * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; */
    popcnt_local = arry[i] - ((arry[i] >> 1) & 0x5555555555555555L);
    popcnt_local = (popcnt_local & 0x3333333333333333L) + ((popcnt_local >> 2) & 0x3333333333333333L);
    popcnt_local = (popcnt_local + (popcnt_local >> 4)) & 0x0F0F0F0F0F0F0F0FL;
    ret += (popcnt_local * 0x0101010101010101L) >> 56;
  }
  return ret;
}

Return_T popcnt_magic_nums_nomul(T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n;
    n = (arry[i] & 0x5555555555555555L) + ((arry[i] >> 1) & 0x5555555555555555L);
    n = (n & 0x3333333333333333L) + ((n >> 2) & 0x3333333333333333L);
    n = (n & 0x0F0F0F0F0F0F0F0FL) + ((n >> 4) & 0x0F0F0F0F0F0F0F0FL);
    n = (n & 0x00FF00FF00FF00FFL) + ((n >> 8) & 0x00FF00FF00FF00FFL);
    n = (n & 0x0000FFFF0000FFFFL) + ((n >> 16) & 0x0000FFFF0000FFFFL);
    //  n = (n & 0x00000000FFFFFFFFL) + ((n >> 32) & 0x00000000FFFFFFFFL);
    n = n + (n >> 32);
    ret += (Return_T) n;
  }
  return ret;
}

#define RUN_AND_REPORT(fn)                                              \
  {                                                                     \
  start_time = clock();                                                 \
  current_answer = fn(data, n);                                         \
  end_time = clock();                                                   \
  time_spent = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;     \
  printf("Runtime of %33s: %6.3f. Answer: %10d\n",                      \
         #fn,                                                           \
         time_spent,                                                    \
         current_answer);                                               \
  }


int main (void) {
  int fd;
  struct stat s = {0};
  uint64_t *data = NULL;
  size_t n;
  clock_t start_time, end_time;
  double time_spent;

  Return_T current_answer;

  fd = open("data.bin", O_RDONLY);
  fstat(fd, &s);
  data = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);

  n = s.st_size / sizeof(T);

  RUN_AND_REPORT(popcnt_naive);
  RUN_AND_REPORT(popcnt_naive_improved);
  RUN_AND_REPORT(popcnt_naive_improved_branching);
  RUN_AND_REPORT(popcnt_kernighan);
  RUN_AND_REPORT(popcnt_builtin);
  RUN_AND_REPORT(popcnt_lookup);
  RUN_AND_REPORT(popcnt_inline_asm);
  RUN_AND_REPORT(popcnt_magic_nums);
  RUN_AND_REPORT(popcnt_naive_unrolled);
  RUN_AND_REPORT(popcnt_magic_nums_nomul);
  
  return 0;
}
