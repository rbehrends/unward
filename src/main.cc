#include "adlib.h"

#include "args.h"
#include "lexer.h"
#include "analyzer.h"
#include "rewrite.h"

#define ROOT_FUNCS A("PTR_BAG", "CONST_PTR_BAG")
#define BLACKLIST A("CHANGED_BAG")

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
    _pwd = CurrentDir();
  }
  Str *operator()(Str *path) {
    if (path->starts_with("/"))
      return path;
    else
      return AbsolutePath(path);
  }
};

StrArr *FindFiles(StrArr *paths) {
#ifdef HAVE_DIRENT_H
  StrArr *result = new StrArr();
  for (Int i = 0; i < paths->len(); i++) {
    StrArr *files = ListFileTree(paths->at(i));
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

FileMap *InputFileMap;
SourceList *InputSources;

StrSet *FuncNames(FuncList *funcs) {
  StrSet *result = new StrSet();
  for (Int i = 0; i < funcs->len(); i++) {
    result->add(funcs->at(i)->name);
  }
  return result;
}

void Main() {
  GCVar(InputFileMap, new FileMap());
  GCVar(InputSources, new SourceList());
  Options *opts = ParseArgs();
  if (opts->InPlace == (opts->OutputDir != NULL)) {
    Error(S("must specify exactly one of --inplace (-i) and --output (-o)"));
  }
  if (opts->OutputDir != NULL && opts->InputFiles->len() != 1) {
    Error(S("If -o is specified as an option, there must be"
            "only one input directory"));
  }
  StrArr *files = FindFiles(opts->InputFiles);
  for (Int i = 0; i < files->len(); i++) {
    Str *file = files->at(i);
    if (!file->ends_with(".h") &&
        !file->ends_with(".c") &&
        !file->ends_with(".cc"))
      continue;
    if (!InputFileMap->contains(file)) {
      SourceFile *source = new SourceFile();
      source->filename = file;
      if (!ReadSource(source)) {
        Warning(S("cannot read file: ")->add(file));
      } else if (!Tokenize(source)) {
        Warning(S("file contains lexical errors: ")->add(file));
      }
      // PrintTokenList(source->tokens);
      InputFileMap->add(file, source);
      InputSources->add(source);
    }
  }
  // Step 1: Find all static inline functions
  FuncList *funcs = FindInlineFunctions(InputSources);
  StrSet *unsafe_names = new StrSet();
  for (Int i = 0; i < funcs->len(); i++) {
    Str *name = funcs->at(i)->name;
    if (name->starts_with("UNSAFE_") || name->starts_with("Unsafe")) {
      unsafe_names->add(name);
      unsafe_names->add(SafeName(name));
    }
  }
  // Step 2: Find out which of those call other functions
  FindCalls(funcs);
  // Step 3: Use the call graph to figure out which functions
  // call any element of ROOT_FUNCS directly or indirectly.
  BitMatrix *calleegraph = BuildCallGraph(funcs, Callees);
  FuncList *wardfuncs =
    FindAllCalls(calleegraph, funcs, ROOT_FUNCS);
  StrSet *wardfuncnames = FuncNames(wardfuncs);
  // Step 4: Find all parts of the code that are surrounded by
  // #ifndef WARD_ENABLED or #if !defined(WARD_ENABLED).
  SectionList *unprotected = FindUnsafeSections(InputSources, wardfuncs);
  // Step 5: Find all static inline functions called from those
  // sections, including indirect calls.
  StrSet *used_funcnames = FindCalls(BuildFuncMap(funcs), unprotected);
  BitMatrix *callergraph = BuildCallGraph(funcs, Callers);
  FuncList *used_funcs = FindAllCalls(callergraph, funcs,
    used_funcnames->items());
  used_funcnames = FuncNames(used_funcs);
  // Step 6: We need to consider all functions that may both trigger
  // a guard AND are called from an unprotected section.
  used_funcnames->intersect_in_place(wardfuncnames);
  // Step 7: Remove false positives; this is right now only CHANGED_BAG(),
  // which we know to be safe and which shouldn't have guards for
  // performance reasons.
  used_funcnames->diff_in_place(new StrSet(BLACKLIST));
  // Step 8: Perform in-memory transformations of the source files.
  SourceList *unsafe_sources = UpdateUnsafeSections(unprotected,
    used_funcnames);
  StrSet *dont_rewrite = new StrSet();
  // do not generate code for already existing unsafe functions
  dont_rewrite->union_in_place(unsafe_names);
  SourceList *unsafe_sources2 = GenUnsafeCode(funcs, used_funcnames,
    dont_rewrite);
  // Step 9: Write files to disk
  if (!opts->OutputDir) {
    // only changed source files
    RewriteSourceFiles(unsafe_sources, opts);
    RewriteSourceFiles(unsafe_sources2, opts);
  } else {
    // write all to the output directory
    RewriteSourceFiles(InputSources, opts);
  }
}
