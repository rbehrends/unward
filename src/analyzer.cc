#include "adlib/lib.h"
#include "adlib/set.h"
#include "adlib/map.h"

#include "lexer.h"
#include "analyzer.h"

static inline Token *GetToken(TokenList *tokens, Int pos) {
  return &tokens->at(pos);
}

static Token *Skip(TokenList *tokens, Int &pos, Word64 symset) {
  Token *token = GetToken(tokens, pos);
  while (TEST(symset, token->sym)) {
    token++;
    pos++;
  }
  return token;
}

static Token *SkipUntil(TokenList *tokens, Int &pos, Word64 symset) {
  symset |= BIT(SymEOF);
  Token *token = GetToken(tokens, pos);
  while (!TEST(symset, token->sym)) {
    token++;
    pos++;
  }
  return token;
}

static Token *SkipToMatch(TokenList *tokens, Int &pos) {
  Token *token = GetToken(tokens, pos);
  Symbol left = token->sym;
  Symbol right = (Symbol)(left+1);
  Int level = 0;
  for (;;) {
    if (token->sym == left) {
      level++;
    } else if (token->sym == right) {
      level--;
      if (level == 0) {
        return token;
      }
    } else if (token->sym == SymEOF) {
      return token;
    }
    token++;
    pos++;
  }
}

FuncList *FindInlineFunctions(SourceFile *source) {
  FuncList *result = new FuncList();
  TokenList *tokens = source->tokens;
  Int pos = 0;
  Int namepos = 0;
  Int start = 0;
  Str *name = NULL;
  for (;;) {
    Token *token = GetToken(tokens, pos);
    if (token->sym == SymEOF) break;
    if (token->sym == SymStatic) {
      start = pos;
      pos++;
      token = Skip(tokens, pos, SymsWS);
      if (token->sym != SymInline) continue;
      token = SkipUntil(tokens, pos, BIT(SymLPar));
      if (token->sym == SymEOF) break;
      if ((token-1)->sym != SymIdent) continue;
      name = (token-1)->str;
      namepos = pos - 1;
      token = SkipUntil(tokens, pos, BIT(SymLBrace)| BIT(SymEOF));
      if (token->sym == SymEOF) break;
      token = SkipToMatch(tokens, pos);
      FuncSpec *fn = new FuncSpec();
      fn->source = source;
      fn->start = start;
      fn->end = pos;
      fn->namepos = namepos;
      fn->name = name;
      result->add(fn);
      pos++;
    } else {
      pos++;
    }
  }
  return result;
}

FuncList *FindInlineFunctions(SourceList *sources) {
  FuncList *result = new FuncList();
  for (Int i = 0; i < sources->len(); i++) {
    SourceFile *source = sources->at(i);
    if (source->filename->ends_with(".h")) {
      result->add(FindInlineFunctions(source));
    }
  }
  return result;
}

void NumberFuncList(FuncList *funcs) {
  for (Int i = 0; i < funcs->len(); i++)
    funcs->at(i)->index = i;
}

PosList *FindCalls(FuncMap *funcmap, SourceFile *source,
    Int start, Int end) {
  PosList *calls = new PosList();
  Int pos = start;
  Int sympos = 0;
  TokenList *tokens = source->tokens;
  SkipUntil(tokens, pos, BIT(SymLBrace));
  while (pos <= end) {
    Token *id = SkipUntil(tokens, pos, BIT(SymIdent));
    if (pos > end)
      break;
    sympos = pos;
    pos++;
    if (!funcmap->contains(id->str))
      continue;
    Token *next = Skip(tokens, pos, SymsWS);
    if (pos > end)
      break;
    if (next->sym == SymLPar) { // is it a call?
      calls->add(sympos);
    }
  }
  return calls;
}

void FindCalls(FuncMap *funcmap, FuncSpec *func) {
  PosList *callpositions =
    FindCalls(funcmap, func->source, func->start, func->end);
  StrSet *callnames = new StrSet();
  for (Int i = 0; i < callpositions->len(); i++) {
    Int pos = callpositions->at(i);
    callnames->add(func->source->tokens->at(pos).str);
  }
  func->calls = new FuncList();
  for (StrSet::Each it(callnames); it; it++) {
    func->calls->add(funcmap->at(*it));
  }
}

FuncMap *BuildFuncMap(FuncList *funcs) {
  FuncMap *funcmap = new FuncMap();
  for (Int i = 0; i < funcs->len(); i++) {
    funcmap->add(funcs->at(i)->name, funcs->at(i));
  }
  return funcmap;
}

void FindCalls(FuncList *funcs) {
  FuncMap *funcmap = BuildFuncMap(funcs);
  for (Int i = 0; i < funcs->len(); i++) {
    funcmap->add(funcs->at(i)->name, funcs->at(i));
  }
  for (Int i = 0; i < funcs->len(); i++) {
    FindCalls(funcmap, funcs->at(i));
  }
}

BitMatrix *BuildCallGraph(FuncList *funcs, CallDirection mode) {
  BitMatrix *result = MakeBitMatrix(funcs->len(), funcs->len());
  for (Int i = 0; i < funcs->len(); i++) {
    FuncSpec *func = funcs->at(i);
    func->index = i;
  }
  for (Int i = 0; i < funcs->len(); i++) {
    FuncSpec *func = funcs->at(i);
    FuncList *calls = func->calls;
    for (Int j = 0; j < calls->len(); j++) {
      FuncSpec *target = calls->at(j);
      switch (mode) {
      case Callers:
        result->at(func->index)->set(target->index);
        break;
      case Callees:
        result->at(target->index)->set(func->index);
        break;
      }
    }
  }
  return TransitiveClosure(result);
}

FuncList *FindAllCalls(BitMatrix *callgraph, FuncList *funcs, StrArr* base) {
  FuncList *result = new FuncList();
  StrSet *basefuncs = new StrSet(base);
  BitSet *set = new BitSet(callgraph->at(0)->len());
  for (Int i = 0; i < callgraph->len(); i++) {
    FuncSpec *func = funcs->at(i);
    if (basefuncs->contains(func->name)) {
      set->union_in_place(callgraph->at(i));
    }
  }
  for (Int i = 0; i < callgraph->len(); i++) {
    if (set->test(i)) {
      result->add(funcs->at(i));
    }
  }
  return result;
}

FuncList *FindAllCalls(BitMatrix *callgraph, FuncList *funcs, FuncList* base) {
  StrArr *names = A();
  for (Int i = 0; i < base->len(); i++) {
    names->add(base->at(i)->name);
  }
  return FindAllCalls(callgraph, funcs, names);
}


SectionSpec *FindUnsafeSections(SourceFile *source) {
  SectionSpec *sec = new SectionSpec();
  TokenList *tokens = source->tokens;
  Arr<bool> *prot_stack = new Arr<bool>();
  bool prot = true;
  sec->source = source;
  sec->start = new PosList();
  sec->end = new PosList();
  for (Int i = 0; i < tokens->len(); i++) {
    Token *token = &tokens->at(i);
    switch (token->sym) {
      case SymPPIf:
        prot_stack->add(prot);
        // Need to parse this properly
        if (token->str->find("WARD_ENABLED") != NOWHERE) {
          prot = false;
          sec->start->add(i);
        }
        break;
      case SymPPElse:
        if (!prot) {
          prot = true;
          sec->end->add(i);
        }
        break;
      case SymPPElif:
        if (!prot) {
          prot = true;
          sec->end->add(i);
        }
      case SymPPEndif:
        if (!prot && prot_stack->last()) {
          sec->end->add(i);
        }
        prot = prot_stack->last();
        prot_stack->pop();
      default:
        break;
    }
  }
  if (sec->start->len() == 0)
    return NULL;
  ensure(sec->start->len() == sec->end->len(),
    "ward sections did not match up");
  return sec;
}

SectionList *FindUnsafeSections(SourceList *sources) {
  SectionList *result = new SectionList();
  for (Int i = 0; i < sources->len(); i++) {
    SectionSpec *sec = FindUnsafeSections(sources->at(i));
    if (sec)
      result->add(sec);
  }
  return result;
}

StrSet *FindCalls(FuncMap *funcmap, SectionList *sections) {
  StrSet *result = new StrSet();
  for (Int i = 0; i < sections->len(); i++) {
    SectionSpec *sec = sections->at(i);
    TokenList *tokens = sec->source->tokens;
    PosList *start = sec->start;
    PosList *end = sec->end;
    PosList *calls = new PosList();
    for (Int j = 0; j < start->len(); j++) {
      calls->add(FindCalls(funcmap, sec->source, start->at(j), end->at(j)));
    }
    for (Int j = 0; j < calls->len(); j++) {
      result->add(tokens->at(calls->at(j)).str);
    }
  }
  return result;
}

