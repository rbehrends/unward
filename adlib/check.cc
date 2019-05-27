#include "lib.h"

static const char *ok = "[OK] ";
static const char *err = "[ERR]";

static int colored = 0;

static void colorize() {
  if (colored)
    return;
  colored = 1;
#ifdef HAVE_ISATTY
  if (isatty(fileno(stdout))) {
    ok = "\033[32m[OK] \033[39m";
    err = "\033[31m[ERR]\033[39m";
  }
#endif
}

void Check(bool cond, const char *message) {
  Str *prog = (new Str(ArgV[0]))->split('/')->last();
  char *name = prog->c_str();
  colorize();
  printf("%-12s %s %s\n", name, cond ? ok : err, message);
}
