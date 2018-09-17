#pragma once

typedef unsigned char Byte;
typedef const char *CString;

#define CmpFunc(T, func) int (*func)(T, T)
#define HashFunc(T, func) Word (*func)(T)
#define TestFunc(T, func) int (*func)(T)
#define MapFunc(T, func) T (*func)(T)
#define FoldFunc(T, A, func) A (*func)(A, T)

#define D(p) (*(p))
#define NUMOF(a) (sizeof(a) / sizeof(a[0]))

#define NOWHERE ((Word) -1)

static inline Word nextPow2(Word n) {
  Word result = 1;
  while (result < n)
    result *= 2;
  return result;
}

template <typename T>
static inline T Min(T a, T b) {
  return a < b ? a : b;
}

template <typename T>
static inline T Max(T a, T b) {
  return a > b ? a : b;
}

int Cmp(Int a, Int b);
int Cmp(Word a, Word b);
