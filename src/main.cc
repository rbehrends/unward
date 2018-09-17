#include "adlib/lib.h"
#include "adlib/map.h"

#include "lexer.h"
#include "analyzer.h"

void Warning(Str *s) {
  printf("warning: %s\n", s->c_str());
}

void Error(Str *s) {
  printf("error: %s\n", s->c_str());
  exit(1);
}

// declarations

typedef Map<Str *, SourceFile *> FileMap;

// debugging functionality

void PrintTokenList(TokenList *tokens) {
  for (Word i = 0; i < tokens->len(); i++) {
    Token token = tokens->item(i);
    printf("%s: %s\n", SymbolNames[token.sym], token.str->c_str());
  }
}
void PrintInlineList(InlineList *funcs) {
  for (Word i = 0; i < funcs->len(); i++) {
    printf("%s\n", funcs->item(i)->name->c_str());
  }
}

// main code

FileMap *InputFiles;
SourceList *InputSources;

void Main() {
  GCVar(InputFiles, new FileMap());
  GCVar(InputSources, new SourceList());
  for (Word i = 0; i < Args->len(); i++) {
    Str *file = Args->item(i);
    if (!InputFiles->contains(file)) {
      SourceFile *source = new SourceFile();
      source->filename = file;
      if (!ReadSource(source)) {
        Warning(S("cannot read file: ")->add(file));
      } else if (!Tokenize(source)) {
        Warning(S("file contains lexical errors: ")->add(file));
      }
      // PrintTokenList(source->tokens);
      InputFiles->add(file, source);
      InputSources->add(source);
    }
  }
  InlineList *funcs = FindInlineFunctions(InputSources);
  // PrintInlineList(funcs);
}
