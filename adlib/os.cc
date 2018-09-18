#include "lib.h"

#include <sys/stat.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

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
  command->add(ShellEscape(prog));
  for (Word i = 0; i < args->len(); i++) {
    command->add(' ')->add(ShellEscape(args->at(i)));
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

void Print(Str *str) {
  printf("%s", str->c_str());
}

void PrintLn(Str *str) {
  printf("%s\n", str->c_str());
}

void PrintErr(Str *str) {
  fprintf(stderr, "%s", str->c_str());
}

void PrintErrLn(Str *str) {
  fprintf(stderr, "%s\n", str->c_str());
}

void Print(const char *str) {
  printf("%s", str);
}

void PrintLn(const char *str) {
  printf("%s\n", str);
}

void PrintErr(const char *str) {
  fprintf(stderr, "%s", str);
}

void PrintErrLn(const char *str) {
  fprintf(stderr, "%s\n", str);
}

void Print(Int i) {
  printf("%" WORD_FMT "d", i);
}

void PrintLn(Int i) {
  printf("%" WORD_FMT "d\n", i);
}

void PrintErr(Int i) {
  fprintf(stderr, "%" WORD_FMT "d", i);
}

void PrintErrLn(Int i) {
  fprintf(stderr, "%" WORD_FMT "d\n", i);
}

Str *Pwd() {
  char *path = getwd(NULL);
  Str *result = new Str(path);
  free(path);
  return result;
}

bool ChDir(const char *path) {
  return !chdir(path);
}

bool ChDir(Str *path) {
  return ChDir(path->c_str());
}

bool FileStat(FileInfo &info, const char *path, bool follow_links) {
  struct stat st;
  if (follow_links) {
    if (lstat(path, &st) < 0)
      return false;
  } else {
    if (stat(path, &st) < 0)
      return false;
  }
  memset(&info, 0, sizeof(info));
  if (S_ISDIR(st.st_mode)) {
    info.is_dir = true;
  } else if (S_ISREG(st.st_mode)) {
    info.is_file = true;
  } else if (S_ISLNK(st.st_mode)) {
    info.is_link = true;
  } else {
    info.is_other = true;
  }
  info.atime = st.st_atime;
  info.mtime = st.st_mtime;
  info.ctime = st.st_ctime;
  info.size = st.st_size;
  return true;
}

bool FileStat(FileInfo &info, Str *path, bool follow_links) {
  return FileStat(info, path->c_str(), follow_links);
}

FileInfo *FileStat(const char *path, bool follow_links) {
  FileInfo info;
  if (FileStat(info, path, follow_links))
    return new FileInfo(info);
  else
    return NULL;
}

FileInfo *FileStat(Str *path, bool follow_links) {
  return FileStat(path, follow_links);
}

StrArr *ReadDir(const char *path) {
#ifdef HAVE_DIRENT_H
  StrArr *result = new StrArr();
  DIR *dir = opendir(path);
  if (!dir)
    return NULL;
  for (;;) {
    struct dirent *entry = readdir(dir);
    if (!entry)
      break;
    if (entry->d_namlen == 1 && entry->d_name[0] == '.')
      continue;
    if (entry->d_namlen == 2 && memcmp(entry->d_name, "..", 2) == 0)
      continue;
    result->add(new Str(entry->d_name, entry->d_namlen));
  }
  closedir(dir);
  return result;
#else
  return NULL;
#endif
}

StrArr *ReadDir(Str *path) {
  return ReadDir(path->c_str());
}

#define FILE_SEP "/"

static void WalkDir(StrArr *acc, const char *path, bool with_dirs) {
  StrArr *files = ReadDir(path);
  if (!files)
    return;
  FileInfo info;
  for (Word i = 0; i < files->len(); i++) {
    Str *newpath = new Str(path);
    if (!newpath->ends_with(FILE_SEP))
      newpath->add(FILE_SEP);
    newpath->add(files->at(i));
    if (FileStat(info, newpath)) {
      if (info.is_dir) {
        newpath->add(FILE_SEP);
        WalkDir(acc, newpath->c_str(), with_dirs);
        if (with_dirs)
          acc->add(newpath);
      } else {
        acc->add(newpath);
      }
    }
  }
}

StrArr *ReadDirRecursive(const char *path, bool with_dirs) {
  StrArr *result = new StrArr();
  FileInfo info;
  if (FileStat(info, path)) {
    if (info.is_dir) {
      Str *dir = new Str(path);
      if (!dir->ends_with(FILE_SEP))
        dir->add(FILE_SEP);
      result->add(dir);
      WalkDir(result, path, with_dirs);
    } else {
      result->add(S(path));
    }
  }
  return result;
}

StrArr *ReadDirRecursive(Str *path, bool with_dirs) {
  return ReadDirRecursive(path->c_str(), with_dirs);
}
