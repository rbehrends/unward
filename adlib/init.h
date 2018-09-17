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

extern Initializer *initializers;

#define INIT(name, code) \
  struct _Init_##name : public Initializer { \
    _Init_##name() \
        : Initializer() { \
      next = initializers; \
      initializers = this; \
    } \
    virtual void init(); \
  } _init_##name; \
  void _Init_##name::init() { \
    code \
  }
