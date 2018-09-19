#include "lib.h"
#include <algorithm>

Str *Str::chomp() {
  if (_len > 0 && _data[_len - 1] == '\n')
    _len--;
  if (_len > 0 && _data[_len - 1] == '\r')
    _len--;
  _data[_len] = '\0';
  return this;
}

StrArr *Str::split(const char *s, Int n) {
  Arr<Int> *parts = new Arr<Int>();
  parts->add(-n);
  for (Int i = 0; i < _len - n; i++) {
    if (_data[i] == s[0] && memcmp(_data + i, s, n)) {
      parts->add(i);
      i += n;
    }
  }
  parts->add(_len);
  StrArr *result = new StrArr(parts->len() - 1);
  for (Int i = 1; i < parts->len(); i++) {
    Int begin = parts->at(i - 1) + n;
    Int end = parts->at(i);
    result->add(new Str(_data + begin, end - begin));
  }
  return result;
}

StrArr *Str::split(Str *sep) {
  return split(sep->_data, sep->_len);
}

StrArr *Str::split(char ch) {
  Int parts = 1;
  for (Int i = 0; i < _len; i++) {
    if (_data[i] == ch)
      parts++;
  }
  StrArr *result = new StrArr(parts);
  Int last = 0;
  for (Int i = 0; i < _len; i++) {
    if (_data[i] == ch) {
      result->add(new Str(_data + last, i - last));
      last = i + 1;
    }
  }
  result->add(new Str(_data + last, _len - last));
  return result;
}

StrArr *Str::split(const char *s) {
  return split(s, strlen(s));
}

StrArr *Str::splitLines() {
  StrArr *result = split('\n');
  for (Int i = 0; i < result->len(); i++) {
    result->at(i)->chomp();
  }
  return result;
}

Str *StrJoin(StrArr *arr, const char *sep, Int n) {
  if (arr->len() == 0)
    return new Str();
  Str *result = new Str(arr->len() * (n + 1));
  result->add(arr->first());
  for (Int i = 1; i < arr->len(); i++) {
    result->add(sep, n);
    result->add(arr->at(i));
  }
  return result;
}

Str *StrJoin(StrArr *arr, const char *sep) {
  return StrJoin(arr, sep, strlen(sep));
}

Str *StrJoin(StrArr *arr, char ch) {
  return StrJoin(arr, &ch, 1);
}

Str *StrJoin(StrArr *arr, Str *sep) {
  return StrJoin(arr, sep->c_str(), sep->len());
}

bool Str::starts_with(const char *s, Int n) {
  if (n > _len)
    return false;
  return memcmp(_data, s, n) == 0;
}

bool Str::starts_with(const char *s) {
  return starts_with(s, strlen(s));
}

bool Str::starts_with(Str *str) {
  return starts_with(str->c_str(), str->len());
}

bool Str::ends_with(const char *s, Int n) {
  if (n > _len)
    return false;
  return memcmp(_data + _len - n, s, n) == 0;
}

bool Str::ends_with(const char *s) {
  return ends_with(s, strlen(s));
}

bool Str::ends_with(Str *str) {
  return ends_with(str->c_str(), str->len());
}

int Cmp(Str *str1, Str *str2) {
  Int len1 = str1->len();
  Int len2 = str2->len();
  int result = memcmp(str1->c_str(), str2->c_str(), Min(len1, len2));
  if (result == 0) {
    if (len1 < len2)
      result = -1;
    else if (len1 > len2)
      result = 1;
  }
  return result;
}

int StrCmp(Str *str1, Str *str2) {
  return Cmp(str1, str2);
}

bool Str::eq(const char *s, Int n) {
  if (n != _len)
    return false;
  return memcmp(_data, s, n) == 0;
}

bool Str::eq(const char *s) {
  return eq(s, strlen(s));
}

bool Str::eq(Str *str) {
  return eq(str->c_str(), str->len());
}

Str *Str::substr(Int start, Int count) {
  require(0 <= start && start < _len && count >= 0 && start + count <= _len,
    "index out of range");
  return new Str(_data + start, count);
}

Int Str::find(char ch, Int from) {
  require(from <= _len, "index out of range");
  for (Int i = from; i < _len; i++) {
    if (_data[i] == ch)
      return i;
  }
  return NOWHERE;
}

Int Str::find(const char *s, Int n, Int from) {
  require(n > 0, "empty string");
  require(from < _len, "index out of range");
  if (n > _len)
    return NOWHERE;
  Int end = _len - n + 1;
  char ch = s[0];
  for (Int i = from; i < end; i++) {
    if (_data[i] == ch) {
      if (memcmp(_data + i, s, n) == 0)
        return i;
    }
  }
  return NOWHERE;
}

Int Str::find(const char *s, Int from) {
  return find(s, strlen(s), from);
}

Int Str::find(Str *str, Int from) {
  return find(str->_data, str->_len, from);
}

Int Str::rfind(char ch) {
  for (Int i = _len - 1; i >= 0; i--) {
    if (_data[i] == ch)
      return i;
  }
  return NOWHERE;
}

Int Str::rfind(const char *s, Int n) {
  require(n > 0, "empty string");
  if (n > _len)
    return NOWHERE;
  Int end = _len - n;
  char ch = s[0];
  for (Int i = end - 1; i >= 0; i--) {
    if (_data[i] == ch) {
      if (memcmp(_data + i, s, n) == 0)
        return i;
    }
  }
  return NOWHERE;
}

Int Str::rfind(const char *s) {
  return rfind(s, strlen(s));
}

Int Str::rfind(Str *str) {
  return rfind(str->_data, str->_len);
}

Str *ToStr(Int x) {
  char buffer[sizeof(LongInt) * 4 + 1];
  sprintf(buffer, "%" LONG_FMT "d", (LongInt) x);
  return S(buffer);
}

Str *ToStr(Word x) {
  char buffer[sizeof(LongWord) * 4 + 1];
  sprintf(buffer, "%" LONG_FMT "u", (LongWord) x);
  return S(buffer);
}

Str *CStrToStr(const char *s) {
  return S(s);
}
