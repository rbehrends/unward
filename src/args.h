#pragma once

struct Options : public GC {
  bool InPlace;
  Str *OutputDir;
  StrArr *InputFiles;
};

Options *ParseArgs();
