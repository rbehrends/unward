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
