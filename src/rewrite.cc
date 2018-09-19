#include "adlib/lib.h"
#include "adlib/map.h"
#include "adlib/set.h"

#include "analyzer.h"
#include "lexer.h"

int CmpFuncSpec(FuncSpec *a, FuncSpec *b) {
  int result = Cmp(a->source->filename, b->source->filename);
  if (!result) {
    result = Cmp(a->start, b->start);
  }
  return result;
}

Str *UnsafeName(Str *name) {
  static Dict *unsafe_names;
  if (!unsafe_names) {
    GCVar(unsafe_names, new Dict());
  }
  Str *result;
  if (!unsafe_names->find(name, result)) {
    bool has_lower_case = false;
    const char *unsafe_prefix = "UNSAFE_";
    for (Int i = 0; i < name->len(); i++) {
      char ch = name->at(i);
      if (ch >= 'a' && ch <= 'z') {
        has_lower_case = true;
        break;
      }
      if (has_lower_case) {
        unsafe_prefix = "Unsafe";
      }
    }
    result = S(unsafe_prefix)->add(name);
    unsafe_names->add(name, result);
  }
  return result;
}

void GenUnsafeCode(FuncSpec *func, StrSet *filter) {
  TokenList *tokens = func->source->tokens;
  Int start = func->start;
  // Include horizontal white space.
  while (start > 0 && tokens->at(start-1).sym == SymWS)
    start--;
  func->start = start;
  Int end = func->end;
  while (tokens->at(end+1).sym == SymWS)
    end++;
  Int funcpos = -1;
  for (Int pos = start; pos <= end; pos++) {
    switch (tokens->at(pos).sym) {
      case SymIdent:
        funcpos = pos;
        break;
      case SymLPar:
        pos = end + 1;
        break;
      default:
        break;
    }
  }
  func->callpositions->add(funcpos);
  TokenList *unsafe_code = tokens->range_incl(start, end);
  PosList *callpositions = func->callpositions;
  for (Int i = 0; i < callpositions->len(); i++) {
    Int pos = callpositions->at(i);
    Str *name = tokens->at(pos).str;
    if (filter->contains(name)) {
      unsafe_code->at(pos-start).str = UnsafeName(name);
    }
  }
  func->unsafe_code = unsafe_code;
}

SourceList *GenUnsafeCode(FuncList *funcs, StrSet *filter) {
  SourceList *result = new SourceList();
  funcs = funcs->sort(CmpFuncSpec);
  for (Int i = 0; i < funcs->len(); i++) {
    FuncSpec *func = funcs->at(i);
    GenUnsafeCode(func, filter);
    if (!func->source->rewritten_funcs) {
      func->source->rewritten_funcs = new FuncList();
      result->add(func->source);
    }
    func->source->rewritten_funcs->add(func);
  }
  return result;
}

void RewriteSourceFile(SourceFile *source) {
  Token nl(SymEOL, Intern("\n", 1));
  FuncList *funcs = source->rewritten_funcs;
  TokenList *tokens = source->tokens;
  TokenList *rewritten = new TokenList();
  rewritten->add(tokens->range_excl(0, funcs->first()->end));
  rewritten->add(nl);
  rewritten->add(nl);
  rewritten->add(funcs->first()->unsafe_code);
  for (Int i = 1; i < funcs->len(); i++) {
    rewritten->add(tokens->range_incl( funcs->at(i-1)->end + 1,
      funcs->at(i)->end));
    rewritten->add(nl);
    rewritten->add(nl);
    rewritten->add(funcs->at(i)->unsafe_code);
  }
  rewritten->add(tokens->range_excl(funcs->last()->end + 1,
    tokens->len()));
  source->rewritten_code = rewritten;

}

Str *TokenToStr(Token token) {
  return token.str;
}

void RewriteSourceFiles(SourceList *sources) {
  for (Int i = 0; i < sources->len(); i++) {
    RewriteSourceFile(sources->at(i));
  }
  for (Int i = 0; i < sources->len(); i++) {
    SourceFile *source = sources->at(i);
    Str *filename = source->filename->clone()->add(".unsafe");
    Str *code = StrJoin(source->rewritten_code->map<Str *>(TokenToStr), "");
    PrintLn(filename);
    WriteFile(filename, code);
  }
}
