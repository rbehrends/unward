#include "adlib.h"

#include "lexer.h"
#include "analyzer.h"
#include "rewrite.h"

void Warning(Str *s) {
  printf("warning: %s\n", s->c_str());
}

void Error(Str *s) {
  printf("error: %s\n", s->c_str());
  exit(1);
}

struct AbsPath : public GC {
  Str *_pwd;
  AbsPath() {
    _pwd = Pwd();
  }
  Str *operator()(Str *path) {
    if (path->starts_with("/"))
      return path;
    else
      return _pwd->clone()->add("/")->add(path);
  }
};

StrArr *FindFiles(StrArr *paths) {
#ifdef HAVE_DIRENT_H
  StrArr *result = new StrArr();
  for (Int i = 0; i < paths->len(); i++) {
    StrArr *files = ReadDirRecursive(paths->at(i));
    result->add(files);
  }
  return result;
#else
  // punt to the Unix find utility
  Str *prog = S("find");
  AbsPath abspath;
  StrArr *args = paths->map<Str *>(abspath);
  args->add(A("-type", "f", "-print"));
  StrArr *lines = ReadProcessLines(prog, args);
  if (!lines)
    return A();
  for (Int i = 0; i < lines->len(); i++)
    lines->item(i)->chomp();
  if (lines->len() > 0 && lines->last()->len() == 0)
    lines->pop();
  return lines;
#endif
}

// declarations

typedef Map<Str *, SourceFile *> FileMap;

// debugging functionality

void PrintTokenList(TokenList *tokens) {
  for (Int i = 0; i < tokens->len(); i++) {
    Token token = tokens->item(i);
    printf("%s: %s\n", SymbolNames[token.sym], token.str->c_str());
  }
}
void PrintFuncList(FuncList *funcs) {
  for (Int i = 0; i < funcs->len(); i++) {
    FuncSpec *func = funcs->at(i);
    PrintLn(func->name);
  }
}

// main code

FileMap *InputFiles;
SourceList *InputSources;

void Main() {
  GCVar(InputFiles, new FileMap());
  GCVar(InputSources, new SourceList());
  StrArr *files = FindFiles(Args);
  for (Int i = 0; i < files->len(); i++) {
    Str *file = files->at(i);
    if (!file->ends_with(".h") && !file->ends_with(".c"))
      continue;
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
  FuncList *funcs = FindInlineFunctions(InputSources);
  FindCalls(funcs);
  BitMatrix *calleegraph = BuildCallGraph(funcs, Callees);
  FuncList *wardfuncs =
    FindAllCalls(calleegraph, funcs, A("PTR_BAG", "CONST_PTR_BAG"));
  StrSet *wardfuncnames = new StrSet();
  for (Int i = 0; i < wardfuncs->len(); i++)
    wardfuncnames->add(wardfuncs->at(i)->name);
  SectionList *unprotected = FindUnsafeSections(InputSources);
  // for (Int i = 0; i < unprotected->len(); i++)
  //   PrintLn(unprotected->at(i)->source->filename);
  StrSet *used_funcnames = FindCalls(BuildFuncMap(funcs), unprotected);
  BitMatrix *callergraph = BuildCallGraph(funcs, Callers);
  FuncList *indirect_used_funcs = FindAllCalls(callergraph, funcs,
    used_funcnames->items());
  // PrintFuncList(funcs);
  for (Int i = 0; i < indirect_used_funcs->len(); i++) {
    Str *name = indirect_used_funcs->at(i)->name;
    used_funcnames->add(name);
  }
  used_funcnames->intersect_in_place(wardfuncnames);
  PrintLn(S(used_funcnames, "\n"));
  SourceList *unsafe_sources = GenUnsafeCode(funcs, used_funcnames);
  RewriteSourceFiles(unsafe_sources);
}
