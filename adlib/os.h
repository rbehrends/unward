#pragma once

Str *ReadFile(FILE *fp);
Str *ReadFile(const char *filename);
Str *ReadFile(Str *filename);
StrArr *ReadLines(const char *filename);
StrArr *ReadLines(Str *filename);
Str *ReadProcess(Str *prog, StrArr *args);
StrArr *ReadProcessLines(Str *prog, StrArr *args);
int WriteFile(FILE *fp, Str *data);
int WriteFile(Str *filename, Str *data);
int WriteFile(const char *filename, Str *data);
int WriteProcess(Str *prog, StrArr *args, Str *data);
int System(Str *prog, StrArr *args);

void Print(Str *str);
void Print(const char *str);
void Print(Int i);
void PrintErr(Str *str);
void PrintErr(const char *str);
void PrintErr(Int i);
void PrintLn(Str *str);
void PrintLn(const char *str);
void PrintLn(Int i);
void PrintErrLn(Str *str);
void PrintErrLn(const char *str);
void PrintErrLn(Int i);

Str *Pwd();
bool ChDir(Str *path); 
bool ChDir(const char *path); 
StrArr *ReadDir(const char *path);
StrArr *ReadDir(Str *path);
StrArr *ReadDir(const char *path);
StrArr *ReadDirRecursive(Str *path, bool with_dirs = false);
StrArr *ReadDirRecursive(const char *path, bool with_dirs = false);

struct FileInfo : PtrFreeGC {
  bool is_dir;
  bool is_file;
  bool is_link;
  bool is_other;
  double atime; // currently no nano-second resolution
  double mtime;
  double ctime;
#ifdef HAVE_OFF_T
  Offset size;
#else
  Int size;
#endif
};

FileInfo *FileStat(Str *path);
bool FileStat(FileInfo & info, const char *path);
bool FileStat(FileInfo & info, Str *path);
FileInfo *FileStat(const char *path);
FileInfo *FileStat(Str *path);
