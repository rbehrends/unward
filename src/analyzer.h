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

struct SectionSpec : public GC {
  SourceFile *source;
  PosList *start;
  PosList *end;
};

typedef Arr<SectionSpec *> SectionList;

FuncList *FindInlineFunctions(SourceFile *source);
FuncList *FindInlineFunctions(SourceList *sources);
void FindCalls(FuncList *funcs);
BitMatrix *BuildCallGraph(FuncList *funcs);
FuncList *FindAllCallers(BitMatrix *callgraph, FuncList *funcs, StrArr *base);
FuncMap *BuildFuncMap(FuncList *funcs);
PosList *FindCalls(FuncMap *funcmap, SourceFile *source, Word start, Word end);
SectionSpec *FindUnsafeSections(SourceFile *source);
SectionList *FindUnsafeSections(SourceList *sources);
StrSet *FindCalls(FuncMap *funcmap, SectionList *sections);
