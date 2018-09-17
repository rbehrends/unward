#pragma once

struct FixStr {
  const char *str;
  Word len;
};

Word Hash(FixStr fs);
int Cmp(FixStr fs1, FixStr fs2);
