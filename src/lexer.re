// vim:set ft=cpp:

#include "adlib/lib.h"
#include "adlib/set.h"
// "fixstr.h" must be included before "map.h"
#include "fixstr.h"
#include "adlib/map.h"

#include "lexer.h"

const char *SymbolNames[] = {
  "SymNone",
  "SymAuto",
  "SymBreak",
  "SymCase",
  "SymChar",
  "SymConst",
  "SymContinue",
  "SymDefault",
  "SymDo",
  "SymDouble",
  "SymElse",
  "SymEnum",
  "SymExtern",
  "SymFloat",
  "SymFor",
  "SymGoto",
  "SymIf",
  "SymInline",
  "SymInt",
  "SymLong",
  "SymRegister",
  "SymReturn",
  "SymShort",
  "SymSigned",
  "SymSizeof",
  "SymStatic",
  "SymStruct",
  "SymSwitch",
  "SymTypedef",
  "SymUnion",
  "SymUnsigned",
  "SymVoid",
  "SymVolatile",
  "SymWhile",
  "SymEXPORT_INLINE",
  "SymIdent",
  "SymLiteral",
  "SymOp",
  "SymSemicolon",
  "SymLPar",
  "SymRPar",
  "SymLBrkt",
  "SymRBrkt",
  "SymLBrace",
  "SymRBrace",
  "SymWS",
  "SymEOL",
  "SymComment",
  "SymBAD",
  "SymPPIf",
  "SymPPElse",
  "SymPPElif",
  "SymPPEndif",
  "SymPPDef",
  "SymPPOther",
  "SymEOF",
};

typedef Map<FixStr, Str *> InternMap;

Str *Intern(const char *ptr, Int len) {
  static InternMap *map = NULL;
  if (!map) {
    GCVar(map, new InternMap());
  }
  FixStr fs;
  fs.str = ptr;
  fs.len = len;
  Str *result = map->get(fs, NULL);
  if (!result) {
    result = new Str(ptr, len);
    // `ptr` above is an interior pointer whose contents may disappear
    // due to GC once the `SourceFile` object containing it is no
    // longer reachable.
    //
    // Therefore, we replace it with `result->c_str()`. This is not only
    // not an interior pointer, but is kept alive by the value that the
    // key is in use for.
    fs.str = result->c_str();
    map->add(fs, result);
  }
  return result;
}

#define PUSH_TOKEN(s) \
  token.sym = s; \
  goto pushtoken;

bool ReadSource(SourceFile *source) {
  if (!source->filedata) {
    source->filedata = ReadFile(source->filename);
  }
  return source->filedata != NULL;
}

bool Tokenize(SourceFile *source) {
  Str *input = source->filedata;
  const char *cursor = input->c_str();
  const char *marker = NULL;
  const char *ctxmarker = NULL;
  bool done = false;
  bool error = false;
  TokenList *result = new TokenList();
  Token token;
  while (!done) {
    const char *last = cursor;
    /*!re2c
    re2c:define:YYCTYPE = "unsigned char";
    re2c:yyfill:enable = 0;
    re2c:define:YYCURSOR = cursor;
    re2c:define:YYMARKER = marker;
    re2c:define:YYCTXMARKER = ctxmarker;

    alpha = [a-zA-Z_];
    digit = [0-9];
    oct = [0-7];
    hex = [0-9a-fA-F];
    floatsuffix = [fFlL]?;
    intsuffix = [uUlL]*;
    exp = 'e' [-+]? digit+;
    squote = ['];
    quote = ["];
    any = [^\000\r\n];
    sp = [ \t\f];
    eol = [\000\r\n];
    nl = "\r" | "\n" | "\r\n";
    postpparg = [^a-zA-Z0-9_\r\n\000];
    pparg = (postpparg any *)?;
    anystr = any \ ["\\];
    anych = any \ ['\\];
    longops = "..." | ">>=" | "<<=" | "+=" | "-=" | "*=" | "/=" | "%="
            | "&=" | "^=" | "|=" | ">>" | "<<" | "++" | "--" | "->"
            | "&&" | "||" | "<=" | ">=" | "==" | "!=";
    esc = "\\";

    "auto"          { PUSH_TOKEN(SymAuto); }
    "break"         { PUSH_TOKEN(SymBreak); }
    "case"          { PUSH_TOKEN(SymCase); }
    "char"          { PUSH_TOKEN(SymChar); }
    "const"         { PUSH_TOKEN(SymConst); }
    "continue"      { PUSH_TOKEN(SymContinue); }
    "default"       { PUSH_TOKEN(SymDefault); }
    "do"            { PUSH_TOKEN(SymDo); }
    "double"        { PUSH_TOKEN(SymDouble); }
    "else"          { PUSH_TOKEN(SymElse); }
    "enum"          { PUSH_TOKEN(SymEnum); }
    "extern"        { PUSH_TOKEN(SymExtern); }
    "float"         { PUSH_TOKEN(SymFloat); }
    "for"           { PUSH_TOKEN(SymFor); }
    "goto"          { PUSH_TOKEN(SymGoto); }
    "if"            { PUSH_TOKEN(SymIf); }
    "inline"        { PUSH_TOKEN(SymInline); }
    "int"           { PUSH_TOKEN(SymInt); }
    "long"          { PUSH_TOKEN(SymLong); }
    "register"      { PUSH_TOKEN(SymRegister); }
    "return"        { PUSH_TOKEN(SymReturn); }
    "short"         { PUSH_TOKEN(SymShort); }
    "signed"        { PUSH_TOKEN(SymSigned); }
    "sizeof"        { PUSH_TOKEN(SymSizeof); }
    "static"        { PUSH_TOKEN(SymStatic); }
    "struct"        { PUSH_TOKEN(SymStruct); }
    "switch"        { PUSH_TOKEN(SymSwitch); }
    "typedef"       { PUSH_TOKEN(SymTypedef); }
    "union"         { PUSH_TOKEN(SymUnion); }
    "unsigned"      { PUSH_TOKEN(SymUnsigned); }
    "void"          { PUSH_TOKEN(SymVoid); }
    "volatile"      { PUSH_TOKEN(SymVolatile); }
    "while"         { PUSH_TOKEN(SymWhile); }
    "EXPORT_INLINE" { PUSH_TOKEN(SymEXPORT_INLINE); }
    alpha (alpha | digit)* { PUSH_TOKEN(SymIdent); }
    '0x' hex+ intsuffix { PUSH_TOKEN(SymLiteral); }
    '0' oct+ intsuffix { PUSH_TOKEN(SymLiteral); }
    digit+ intsuffix { PUSH_TOKEN(SymLiteral); }
    "L"? squote (esc any anych* | anych) squote { PUSH_TOKEN(SymLiteral); }
    "L"? quote (esc any | anystr)* quote { PUSH_TOKEN(SymLiteral); }
    digit+ exp floatsuffix { PUSH_TOKEN(SymLiteral); }
    digit* "." digit+ exp? floatsuffix { PUSH_TOKEN(SymLiteral); }
    digit+ "." digit* exp? floatsuffix { PUSH_TOKEN(SymLiteral); }
    "(" { PUSH_TOKEN(SymLPar); }
    ")" { PUSH_TOKEN(SymRPar); }
    "[" { PUSH_TOKEN(SymLBrkt); }
    "]" { PUSH_TOKEN(SymRBrkt); }
    "{" { PUSH_TOKEN(SymLBrace); }
    "}" { PUSH_TOKEN(SymRBrace); }
    [-.&!~+*%/<>^|?:=,] { PUSH_TOKEN(SymOp); }
    longops { PUSH_TOKEN(SymOp); }
    ";" { PUSH_TOKEN(SymSemicolon); }
    "//" any+ { PUSH_TOKEN(SymComment); }
    "/" "*" { goto comment; }
    nl { PUSH_TOKEN(SymEOL); }
    "\\" sp* / nl { PUSH_TOKEN(SymWS); }
    sp+ { PUSH_TOKEN(SymWS); }
    "#" sp*  "if" pparg / eol { PUSH_TOKEN(SymPPIf); }
    "#" sp* "ifdef" pparg / eol { PUSH_TOKEN(SymPPIf); }
    "#" sp* "ifndef" pparg / eol { PUSH_TOKEN(SymPPIf); }
    "#" sp* "else" pparg / eol { PUSH_TOKEN(SymPPElse); }
    "#" sp* "elif" pparg / eol { PUSH_TOKEN(SymPPElif); }
    "#" sp* "endif" pparg / eol { PUSH_TOKEN(SymPPEndif); }
    "#" sp* "define" pparg / eol { PUSH_TOKEN(SymPPDef); }
    "#" sp* "undefine" pparg / eol { PUSH_TOKEN(SymPPDef); }
    "#" any* / eol { PUSH_TOKEN(SymPPOther); }
    "\000" { done = true; continue; }
    any { error = true; PUSH_TOKEN(SymBAD); }
    * { done = true; continue; }
    */
    comment:
    /*!re2c
    "*" "/" { PUSH_TOKEN(SymComment); }
    [^\000] { goto comment; }
    "\000" { done = true; PUSH_TOKEN(SymComment); }
    */
    pushtoken:
      token.str = Intern(last, cursor - last);
      result->add(token);
  }
  token.sym = SymEOF;
  token.str = S("");
  result->add(token);
  source->tokens = result;
  return !error;
}
