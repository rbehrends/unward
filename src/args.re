// vim:set ft=cpp:

#include "adlib/lib.h"
#include "args.h"

void Help() {
  PrintLn("unward [OPTIONS] files-or-directories ...");
  PrintLn("");
  PrintLn("  -o  --output DIR    output generated code to DIR");
  PrintLn("  -i  --inplace       rewrite source files in place");
  PrintLn("  -h  --help          display this message");
  PrintLn("");
}

Options* ParseArgs() {
  Options *opts = new Options();
  opts->InputFiles = new StrArr();
  char mode = '\0';
  for (Int i = 0; i < Args->len(); i++) {
    Str *arg = Args->at(i);
    if (mode == 'o') {
      opts->OutputDir = arg->clone();
      mode = '0';
      continue;
    }
    char *pos = arg->c_str();
    char *marker;
    /*!re2c
      re2c:define:YYCTYPE = char;
      re2c:define:YYCURSOR = pos;
      re2c:define:YYMARKER = marker;
      re2c:yyfill:enable = 0;

      end = "\000";
      any = [^\000];
      "--output" end { mode = 'o'; continue; }
      "-o" end { mode = 'o'; continue; }
      "-o" (any +) end { opts->OutputDir = arg->range_excl(2, arg->len()); }
      "--inplace" end { opts->InPlace = true; continue; }
      "-i" end { opts->InPlace = true; continue; }
      "--help" end { Help(); exit(0); }
      "-h" end { Help(); exit(0); }
      "-" {
            PrintLn(S("Invalid option: ")->add(arg));
	    PrintLn("");
            Help();
            exit(1);
          }
      * {
        opts->InputFiles->add(arg);
	continue;
      }
    */
  }
  if (mode) {
    PrintLn("Missing option argument");
    PrintLn("");
    Help();
    exit(1);
  }
  return opts;
}
