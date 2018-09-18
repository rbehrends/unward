#pragma once

#include "adlib/bitset.h"

#include "lexer.h"

struct FuncSpec;

typedef Arr<FuncSpec *> FuncList;
typedef Map<Str *, FuncSpec *> FuncMap;

struct FuncSpec : public GC {
  Word index;
  SourceFile *source;
  Str *name;
  FuncList *calls;
  Word start, end;
  Word namepos;
};

typedef Arr<Word> PosList;

FuncList *FindInlineFunctions(SourceFile *source);
FuncList *FindInlineFunctions(SourceList *sources);
void FindCalls(FuncList *funcs);
BitMatrix *BuildCallGraph(FuncList *funcs);
FuncList *FindAllCallers(BitMatrix *callgraph, FuncList *funcs, StrArr *base);
PosList *FindCalls(FuncMap *funcmap, SourceFile *source, Word start, Word end);
