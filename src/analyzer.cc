#include "adlib/lib.h"
#include "adlib/set.h"
#include "adlib/map.h"

#include "lexer.h"
#include "analyzer.h"

static inline Token *GetToken(TokenList *tokens, Word pos) {
  return &tokens->item(pos);
}

static Token *Skip(TokenList *tokens, Word &pos, Word64 symset) {
  Token *token = GetToken(tokens, pos);
  while (TEST(symset, token->sym)) {
    token++;
    pos++;
  }
  return token;
}

static Token *SkipUntil(TokenList *tokens, Word &pos, Word64 symset) {
  symset |= BIT(SymEOF);
  Token *token = GetToken(tokens, pos);
  while (!TEST(symset, token->sym)) {
    token++;
    pos++;
  }
  return token;
}

static Token *SkipToMatch(TokenList *tokens, Word &pos) {
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
  Word pos = 0;
  Word namepos = 0;
  Word start = 0;
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
  for (Word i = 0; i < sources->len(); i++) {
    SourceFile *source = sources->item(i);
    if (source->filename->ends_with(".h")) {
      result->add(FindInlineFunctions(source));
    }
  }
  return result;
}

PosList *FindCalls(FuncMap *funcmap, SourceFile *source,
    Word start, Word end) {
  PosList *calls = new PosList();
  Word pos = start;
  Word sympos = 0;
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
  Arr<Word> *callpositions =
    FindCalls(funcmap, func->source, func->start, func->end);
  StrSet *callnames = new StrSet();
  for (Word i = 0; i < callpositions->len(); i++) {
    Word pos = callpositions->at(i);
    callnames->add(func->source->tokens->at(pos).str);
  }
  func->calls = new FuncList();
  for (StrSet::Each it(callnames); it; it++) {
    func->calls->add(funcmap->at(*it));
  }
}

void FindCalls(FuncList *funcs) {
  FuncMap *funcmap = new FuncMap();
  for (Word i = 0; i < funcs->len(); i++) {
    funcmap->add(funcs->at(i)->name, funcs->at(i));
  }
  for (Word i = 0; i < funcs->len(); i++) {
    FindCalls(funcmap, funcs->at(i));
  }
}

BitMatrix *BuildCallGraph(FuncList *funcs) {
  BitMatrix *result = MakeBitMatrix(funcs->len(), funcs->len());
  for (Word i = 0; i < funcs->len(); i++) {
    FuncSpec *func = funcs->at(i);
    func->index = i;
  }
  for (Word i = 0; i < funcs->len(); i++) {
    FuncSpec *func = funcs->at(i);
    FuncList *calls = func->calls;
    for (Word j = 0; j < calls->len(); j++) {
      FuncSpec *target = calls->at(j);
      result->at(target->index)->set(func->index);
    }
  }
  return TransitiveClosure(result);
}

FuncList *FindAllCallers(BitMatrix *callgraph, FuncList *funcs, StrArr* base) {
  FuncList *result = new FuncList();
  StrSet *basefuncs = new StrSet(base);
  BitSet *set = new BitSet(callgraph->at(0)->len());
  for (Word i = 0; i < callgraph->len(); i++) {
    FuncSpec *func = funcs->at(i);
    if (basefuncs->contains(func->name)) {
      set->union_in_place(callgraph->at(i));
    }
  }
  for (Word i = 0; i < callgraph->len(); i++) {
    if (set->test(i)) {
      result->add(funcs->at(i));
    }
  }
  return result;
}
