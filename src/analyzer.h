#pragma once

#include "lexer.h"

struct InlineFunc : public GC {
  SourceFile *source;
  Str *name;
  Word start, end;
  Word namepos;
};

typedef Arr<InlineFunc *> InlineList;

InlineList *FindInlineFunctions(SourceFile *source);
InlineList *FindInlineFunctions(SourceList *sources);

