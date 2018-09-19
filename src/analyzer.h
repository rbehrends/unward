#pragma once

#include "adlib/bitset.h"

#include "lexer.h"

struct FuncSpec;

typedef Arr<FuncSpec *> FuncList;
typedef Map<Str *, FuncSpec *> FuncMap;
typedef Arr<Int> PosList;


struct FuncSpec : public GC {
  Int index;
  SourceFile *source;
  Str *name;
  FuncList *calls;
  Int start, end;
  Int namepos;
  PosList *callpositions;
  TokenList *unsafe_code;
};

struct SectionSpec : public GC {
  SourceFile *source;
  PosList *start;
  PosList *end;
  PosList *callpositions;
};

enum CallDirection { Callees, Callers };

typedef Arr<SectionSpec *> SectionList;

void NumberFuncList(FuncList *funcs);
FuncList *FindInlineFunctions(SourceFile *source);
FuncList *FindInlineFunctions(SourceList *sources);
void FindCalls(FuncList *funcs);
BitMatrix *BuildCallGraph(FuncList *funcs, CallDirection mode);
FuncMap *BuildFuncMap(FuncList *funcs);
PosList *FindCalls(FuncMap *funcmap, SourceFile *source, Int start, Int end);
SectionList *FindUnsafeSections(SourceList *sources, FuncList *funcs);
StrSet *FindCalls(FuncMap *funcmap, SectionList *sections);
FuncList *FindAllCalls(BitMatrix *callgraph, FuncList *funcs, StrArr *base);
FuncList *FindAllCalls(BitMatrix *callgraph, FuncList *funcs, FuncList *base);
