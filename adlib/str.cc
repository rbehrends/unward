#include "lib.h"
#include <algorithm>

Str *Str::chomp() {
  if (_len > 0 && _data[_len - 1] == '\n')
    _len--;
  if (_len > 0 && _data[_len - 1] == '\r')
    _len--;
  return this;
}

StrArr *Str::split(const char *s, Word n) {
  Arr<Word> *parts = new Arr<Word>();
  parts->add(-n);
  for (Word i = 0; i < _len - n; i++) {
    if (_data[i] == s[0] && memcmp(_data + i, s, n)) {
      parts->add(i);
      i += n;
    }
  }
  parts->add(_len);
  StrArr *result = new StrArr(parts->len() - 1);
  for (Word i = 1; i < parts->len(); i++) {
    Word begin = D(parts)[i - 1] + n;
    Word end = D(parts)[i];
    result->add(new Str(_data + begin, end - begin));
  }
  return result;
}

StrArr *Str::split(Str *sep) {
  return split(sep->_data, sep->_len);
}

StrArr *Str::split(char ch) {
  Word parts = 1;
  for (Word i = 0; i < _len; i++) {
    if (_data[i] == ch)
      parts++;
  }
  StrArr *result = new StrArr(parts);
  Word last = 0;
  for (Word i = 0; i < _len; i++) {
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
  for (Word i = 0; i < result->len(); i++) {
    D(result)[i]->chomp();
  }
  return result;
}

Str *StrJoin(StrArr *arr, const char *sep, Word n) {
  Str *result = new Str(arr->len() * (n + 1));
  result->add(D(arr)[0]);
  for (Word i = 1; i < arr->len(); i++) {
    result->add(sep, n);
    result->add(D(arr)[i]);
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

bool Str::startsWith(const char *s, Word n) {
  if (n > _len)
    return false;
  return memcmp(_data, s, n) == 0;
}

bool Str::startsWith(const char *s) {
  return startsWith(s, strlen(s));
}

bool Str::startsWith(Str *str) {
  return startsWith(str->c_str(), str->len());
}

bool Str::endsWith(const char *s, Word n) {
  if (n > _len)
    return false;
  return memcmp(_data + _len - n, s, n) == 0;
}

bool Str::endsWith(const char *s) {
  return endsWith(s, strlen(s));
}

bool Str::endsWith(Str *str) {
  return endsWith(str->c_str(), str->len());
}

int Cmp(Str *str1, Str *str2) {
  Word len1 = str1->len();
  Word len2 = str2->len();
  int result = memcmp(str1->c_str(), str2->c_str(), Min(len1, len2));
  if (result == 0) {
    if (len1 < len2)
      result = -1;
    else if (len1 > len2)
      result = 1;
  }
  return result;
}

bool Str::eq(const char *s, Word n) {
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

Str *Str::substr(Word start, Word count) {
  require(start + count < _len, "index out of range");
  return new Str(_data + start, count);
}

Word Str::find(char ch, Word from) {
  require(from < _len, "index out of range");
  for (Word i = from; i < _len; i++) {
    if (_data[i] == ch)
      return i;
  }
  return NOWHERE;
}

Word Str::find(const char *s, Word n, Word from) {
  require(n > 0, "empty string");
  require(from < _len, "index out of range");
  Word end = _len - n;
  char ch = s[0];
  for (Word i = from; i < end; i++) {
    if (_data[i] == ch) {
      if (memcmp(_data + i, s, n) == 0)
        return i;
    }
  }
  return NOWHERE;
}

Word Str::find(const char *s, Word from) {
  return find(s, strlen(s), from);
}

Word Str::find(Str *str, Word from) {
  return find(str->_data, str->_len, from);
}

Word Str::rfind(char ch) {
  for (Int i = _len - 1; i >= 0; i--) {
    if (_data[i] == ch)
      return i;
  }
  return NOWHERE;
}

Word Str::rfind(const char *s, Word n) {
  require(n > 0, "empty string");
  Word end = _len - n;
  char ch = s[0];
  for (Int i = end - 1; i >= 0; i--) {
    if (_data[i] == ch) {
      if (memcmp(_data + i, s, n) == 0)
        return i;
    }
  }
  return NOWHERE;
}

Word Str::rfind(const char *s) {
  return rfind(s, strlen(s));
}

Word Str::rfind(Str *str) {
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
