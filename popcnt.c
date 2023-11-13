#include <stdint.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#if defined(__x86_64__)

#define X86_64

#endif


#define T uint64_t
#define Return_T uint64_t // can hold the popcnt of up to 2**26 of T
#define BIT_WIDTH (sizeof(T) * CHAR_BIT)
#define bit_counter unsigned char

Return_T popcnt_naive(const T arry[], size_t n_elts) {
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

Return_T popcnt_naive_branching(const T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n = arry[i];
    for (bit_counter j = 0; j < BIT_WIDTH; j++) {
      if (n & 0x01) {
        ret++;
      }
      n >>= 1;
    }
  }
  return ret;
}

#define DO8(stmt) {                             \
    stmt;                                       \
    stmt;                                       \
    stmt;                                       \
    stmt;                                       \
    stmt;                                       \
    stmt;                                       \
    stmt;                                       \
    stmt;                                       \
  }

Return_T popcnt_naive_inner_unrolled(const T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n = arry[i];
    DO8(ret += n & 0x01L; n >>= 1); //  0-7
    DO8(ret += n & 0x01L; n >>= 1); //  8-15
    DO8(ret += n & 0x01L; n >>= 1); // 16-23
    DO8(ret += n & 0x01L; n >>= 1); // 24-31
    DO8(ret += n & 0x01L; n >>= 1); // 32-39
    DO8(ret += n & 0x01L; n >>= 1); // 40-47
    DO8(ret += n & 0x01L; n >>= 1); // 48-55
    DO8(ret += n & 0x01L; n >>= 1); // 56-63
  }
  return ret;
}

Return_T popcnt_naive_early_exit(const T arry[], size_t n_elts) {
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

Return_T popcnt_naive_early_exit_branching(const T arry[], size_t n_elts) {
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

Return_T popcnt_kernighan(const T arry[], size_t n_elts) {
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

Return_T popcnt_lookup(const T arry[], size_t n_elts) {
  Return_T ret = 0;
  uint8_t *arry_reinterpret = (uint8_t *) arry;
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

  for (size_t i = 0; i < (n_elts * (sizeof(T) / sizeof(uint8_t))); i++) {
    ret += byte_bit_table[arry_reinterpret[i]];
  }
  return ret;
}

Return_T popcnt_magic_nums(const T arry[], size_t n_elts) {
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

Return_T popcnt_magic_nums_nomul(const T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T n;
    n = (arry[i] & 0x5555555555555555L) + ((arry[i] >> 1) & 0x5555555555555555L);
    n = (n & 0x3333333333333333L) + ((n >> 2) & 0x3333333333333333L);
    n = (n & 0x0F0F0F0F0F0F0F0FL) + ((n >> 4) & 0x0F0F0F0F0F0F0F0FL);
    n = (n & 0x00FF00FF00FF00FFL) + ((n >> 8) & 0x00FF00FF00FF00FFL);
    n = (n & 0x0000FFFF0000FFFFL) + ((n >> 16) & 0x0000FFFF0000FFFFL);
    /* n = (n & 0x00000000FFFFFFFFL) + ((n >> 32) & 0x00000000FFFFFFFFL); */
    n = (n + (n >> 32)) & 0x00000000FFFFFFFFL;
    ret += n;
  }
  return ret;
}

Return_T popcnt_builtin(const T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    ret += __builtin_popcountll(arry[i]);
  }
  return ret;
}

#ifdef X86_64
Return_T popcnt_inline_asm(const T arry[], size_t n_elts) {
  Return_T ret = 0;
  for (size_t i = 0; i < n_elts; i++) {
    register T local_popcnt;
    asm volatile ("popcnt %1, %0"
                  : "=r" (local_popcnt)
                  : "r" (arry[i]));
    ret += local_popcnt;
  }
  return ret;
}

Return_T popcnt_inline_asm_unrolled(const T arry[], size_t n_elts) {
  register T local_popcnt[4] = { 0 };
  for (size_t i = 0; i < n_elts; i += 4) {
    register T buf[4]  = { arry[i], arry[i + 1], arry[i+2], arry[i+3]};
    asm volatile ("popcnt %4, %4\n\t"
                  "addq   %4, %0\n\t"
                  "popcnt %5, %5\n\t"
                  "addq   %5, %1\n\t"
                  "popcnt %6, %6\n\t"
                  "addq   %6, %2\n\t"
                  "popcnt %7, %7\n\t"
                  "addq   %7, %3\n\t"
                  : "+r" (local_popcnt[0]), "+r" (local_popcnt[1]), "+r" (local_popcnt[2]), "+r" (local_popcnt[3])
                  : "r" (buf[0]), "r" (buf[1]), "r" (buf[2]), "r" (buf[3]));
  }
  return local_popcnt[0] + local_popcnt[1] + local_popcnt[2] + local_popcnt[3];
}
#endif

void run_and_report(Return_T fn(const T[] , size_t), T data[], size_t n, const char *fn_name) {
  clock_t start_time, end_time;
  double time_spent;
  Return_T current_answer;

  start_time = clock();
  current_answer = fn(data, n);
  end_time = clock();
  time_spent = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
  printf("Runtime of %40s: %6.3f. Answer: %10ld\n", fn_name, time_spent, current_answer);
}

int main (void) {
  int fd;
  struct stat s = {0};
  uint64_t *data = NULL;
  size_t n;

  fd = open("data.bin", O_RDONLY);
  fstat(fd, &s);
  data = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED | MAP_POPULATE, fd, 0);

  n = s.st_size / sizeof(T);

#define RUN_AND_REPORT(fn) run_and_report(fn, data, n, #fn)

  RUN_AND_REPORT(popcnt_naive);
  RUN_AND_REPORT(popcnt_naive_branching);
  RUN_AND_REPORT(popcnt_naive_inner_unrolled);
  RUN_AND_REPORT(popcnt_naive_early_exit);
  RUN_AND_REPORT(popcnt_naive_early_exit_branching);
  RUN_AND_REPORT(popcnt_kernighan);
  RUN_AND_REPORT(popcnt_lookup);
  RUN_AND_REPORT(popcnt_magic_nums);
  RUN_AND_REPORT(popcnt_magic_nums_nomul);
  RUN_AND_REPORT(popcnt_builtin);
#ifdef X86_64
  RUN_AND_REPORT(popcnt_inline_asm);
  RUN_AND_REPORT(popcnt_inline_asm_unrolled);
#endif

  return 0;
}
