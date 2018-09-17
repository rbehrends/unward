#include "adlib/lib.h"
#include "adlib/map.h"

#include "lexer.h"

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

// main code

FileMap *InputSource;
StrArr *InputFiles;

void Main() {
  GCVar(InputSource, new FileMap());
  GCVar(InputFiles, new StrArr());
  for (Word i = 0; i < Args->len(); i++) {
    Str *file = Args->item(i);
    if (!InputSource->contains(file)) {
      SourceFile *source = new SourceFile();
      source->filename = file;
      if (!ReadSource(source)) {
        Warning(S("cannot read file: ")->add(file));
      } else if (!Tokenize(source)) {
        Warning(S("file contains lexical errors: ")->add(file));
      }
      // PrintTokenList(source->tokens);
      InputSource->add(file, source);
      InputFiles->add(file);
    }
  }
}
