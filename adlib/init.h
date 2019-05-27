#pragma once

extern int ArgC;
extern char **ArgV;
extern Str *ProgName;
extern StrArr *Args;

struct Initializer {
  Initializer *next;
  virtual void init() = 0;
  Initializer() {
    next = NULL;
  }
};

void InitSystem();

#define MAX_INIT_PRIO 9

namespace AdLib {
  extern Initializer *initializers[MAX_INIT_PRIO+2];
}

#define INIT_PRIO(name, prio, code) \
  namespace AdLib { namespace Init { \
    struct InitSection_##name : public Initializer { \
      InitSection_##name() \
          : Initializer() { \
        next = initializers[prio+1]; \
        initializers[prio+1] = this; \
      } \
      virtual void init(); \
    } init_section_##name; \
    void InitSection_##name::init() { \
      code \
    } \
  } }

#define INIT(name, code) INIT_PRIO(name, 0, code)
