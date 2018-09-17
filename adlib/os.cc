#include "lib.h"

Str *ReadFile(FILE *fp) {
  if (fp == NULL)
    return NULL;
  int ch;
  Str *result = new Str(1024);
  while ((ch = getc(fp)) != EOF) {
    result->add(ch);
  }
  return result;
}

Str *ReadFile(const char *filename) {
  FILE *fp = fopen(filename, "r");
  Str *result = ReadFile(fp);
  fclose(fp);
  return result;
}

Str *ReadFile(Str *filename) {
  return ReadFile(filename->c_str());
}

int WriteFile(FILE *fp, Str *data) {
  char *p = data->c_str();
  Word len = data->len();
  while (len != 0) {
    Word written = fwrite(p, 1, len, fp);
    if (written == 0)
      return 0;
    len -= written;
    p += written;
  }
  return 1;
}

int WriteFile(const char *filename, Str *data) {
  FILE *fp = fopen(filename, "w");
  if (!fp)
    return 0;
  int success = WriteFile(fp, data);
  fclose(fp);
  return success;
}

int WriteFile(Str *filename, Str *data) {
  return WriteFile(filename->c_str(), data);
}

StrArr *ReadLines(const char *filename) {
  Str *contents = ReadFile(filename);
  return contents->splitLines();
}

StrArr *ReadLines(Str *filename) {
  return ReadLines(filename->c_str());
}

static char safe_chars[256];

void InitSafeChars() {
  for (int i = 'a'; i <= 'z'; i++)
    safe_chars[i] = 1;
  for (int i = 'A'; i <= 'Z'; i++)
    safe_chars[i] = 1;
  for (int i = '0'; i <= '9'; i++)
    safe_chars[i] = 1;
  const char *p = ",._+:@%/-";
  while (*p)
    safe_chars[*p++] = 1;
}

INIT(_AdLibOS, InitSafeChars(););

Str *ShellEscape(Str *arg) {
  int safe = 1;
  for (Str::Each it(arg); it; ++it) {
    if (!safe_chars[*it]) {
      safe = 0;
      break;
    }
  }
  if (safe)
    return arg;
  Str *result = new Str(arg->len());
  result->add('\'');
  for (Str::Each it(arg); it; ++it) {
    if (*it == '\'')
      result->add("'\\''");
    else
      result->add(*it);
  }
  result->add('\'');
  return result;
}

Str *BuildCommand(Str *prog, StrArr *args) {
  Str *command = new Str(1024);
  D(command) << ShellEscape(prog);
  for (Word i = 0; i < args->len(); i++) {
    D(command) << ' ' << ShellEscape(D(args)[i]);
  }
  return command;
}

Str *ReadProcess(Str *prog, StrArr *args) {
  Str *command = BuildCommand(prog, args);
  FILE *pipe = popen(command->c_str(), "r");
  if (!pipe)
    return NULL;
  Str *result = ReadFile(pipe);
  pclose(pipe);
  return result;
}

StrArr *ReadProcessLines(Str *prog, StrArr *args) {
  Str *output = ReadProcess(prog, args);
  return output->splitLines();
}

int WriteProcess(Str *prog, StrArr *args, Str *data) {
  Str *command = BuildCommand(prog, args);
  FILE *pipe = popen(command->c_str(), "w");
  if (!pipe)
    return 0;
  int success = WriteFile(pipe, data);
  pclose(pipe);
  return success;
}

int System(Str *prog, StrArr *args) {
  Str *command = BuildCommand(prog, args);
  int result = system(command->c_str());
  if (result >= 256)
    result >>= 8;
  return result;
}
