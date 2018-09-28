#pragma once

struct Options : public GC {
  bool InPlace;
  bool LineDirs;
  Str *OutputDir;
  StrArr *InputFiles;
};

Options *ParseArgs();
