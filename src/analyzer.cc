#include "adlib/lib.h"
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

InlineList *FindInlineFunctions(SourceFile *source) {
  InlineList *result = new InlineList();
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
      InlineFunc *fn = new InlineFunc();
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

InlineList *FindInlineFunctions(SourceList *sources) {
  InlineList *result = new InlineList();
  for (Word i = 0; i < sources->len(); i++) {
    SourceFile *source = sources->item(i);
    if (source->filename->ends_with(".h")) {
      result->add(FindInlineFunctions(source));
    }
  }
  return result;
}
