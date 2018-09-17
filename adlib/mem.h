#pragma once

class GC {
  public:
  void *operator new(size_t size);
  void operator delete(void *p) {
  }
};

class PtrFreeGC {
  public:
  void *operator new(size_t size);
  void operator delete(void *p) {
  }
};

template <typename T>
void GCVar(T &var) {
  T *start = &var;
  T *end = start + 1;
#ifndef USE_BOEHM_GC
  GC_add_roots(start, end);
#endif
}

template <typename T>
void GCVar(T &var, T val) {
  T *start = &var;
  T *end = start + 1;
#ifndef USE_BOEHM_GC
  GC_add_roots(start, end);
#endif
  var = val;
}
