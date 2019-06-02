#pragma once

enum Symbol {
  SymNone,
  SymAuto,
  SymBreak,
  SymCase,
  SymChar,
  SymConst,
  SymContinue,
  SymDefault,
  SymDo,
  SymDouble,
  SymElse,
  SymEnum,
  SymExtern,
  SymFloat,
  SymFor,
  SymGoto,
  SymIf,
  SymInline,
  SymInt,
  SymLong,
  SymRegister,
  SymReturn,
  SymShort,
  SymSigned,
  SymSizeof,
  SymStatic,
  SymStruct,
  SymSwitch,
  SymTypedef,
  SymUnion,
  SymUnsigned,
  SymVoid,
  SymVolatile,
  SymWhile,
  SymEXPORT_INLINE,
  SymIdent,
  SymLiteral,
  SymOp,
  SymSemicolon,
  SymLPar,
  SymRPar,
  SymLBrkt,
  SymRBrkt,
  SymLBrace,
  SymRBrace,
  SymWS,
  SymEOL,
  SymComment,
  SymBAD,
  SymPPIf,
  SymPPElse,
  SymPPElif,
  SymPPEndif,
  SymPPDef,
  SymPPOther,
  SymEOF,
};

extern const char *SymbolNames[];

#define BIT(n) (((Word64) 1) << (n))
// from 0..n inclusive
#define ALLBITS(n) (BIT(n+1)-1)
// inclusive range
#define BITRANGE(m, n) (ALLBITS(n) & ~ALLBITS(m-1))
#define TEST(set, n) (((set) & BIT(n)) != (Word64) 0)

static const Word64 SymsWS =
  BIT(SymWS) | BIT(SymEOL) | BIT(SymComment);
static const Word64 SymsKeyword =
  BITRANGE(SymNone+1, SymIdent-1);
static const Word64 SymsPunct =
  BIT(SymSemicolon) | BIT(SymLPar) | BIT(SymRPar) |
  BIT(SymLBrkt) | BIT(SymRBrkt) | BIT(SymLBrace) | BIT(SymRBrace);

struct Token : public GC {
  Token() {
    sym = SymNone;
  }
  Token(Symbol _sym, Str *_str) {
    sym = _sym;
    str = _str;
  }
  Symbol sym;
  Str *str;
};

typedef Arr<Token> TokenList;

struct FuncSpec;

struct SourceFile : public GC {
  Str *filename;
  Str *filedata;
  TokenList *tokens;
  bool rewritten;
  Arr<Int> *unsafe_calls;
  Arr<FuncSpec *> *rewritten_funcs;
  TokenList *rewritten_code;
};

typedef Arr<SourceFile *> SourceList;

Str *Intern(const char *str, Int len);
bool ReadSource(SourceFile *source);
bool Tokenize(SourceFile *source);
