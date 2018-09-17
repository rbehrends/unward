#pragma once

class Str;

typedef Arr<Str *> StrArr;

class Str : public GC {
  private:
  Word _len;
  Word _cap;
  char *_data;
  void resize(Word newcap) {
    char *newdata = (char *) GC_MALLOC_ATOMIC(newcap + 1);
    memcpy(newdata, _data, _len);
    _data = newdata;
    _cap = newcap;
  }
  void init(Word cap) {
    _len = 0;
    _cap = cap;
    // atomic memory is not zeroed by default.
    _data = (char *) GC_MALLOC_ATOMIC(cap + 1);
    _data[0] = '\0';
  }
  void init(const char *s, Word len) {
    _len = _cap = len;
    // atomic memory is not zeroed by default.
    _data = (char *) GC_MALLOC_ATOMIC(_cap + 1);
    memcpy(_data, s, len);
    _data[len] = '\0';
  }

  public:
  class Each {
private:
    Str *_str;
    Word _i;

public:
    Each(Str *str) {
      _str = str;
      _i = 0;
    }
    operator bool() {
      return _i < _str->_len;
    }
    void operator++() {
      _i++;
    }
    void operator++(int dummy) {
      _i++;
    }
    char &operator*() {
      return _str->_data[_i];
    }
    char operator->() {
      return _str->_data[_i];
    }
  };
  Str(const char *s) {
    init(s, strlen(s));
  }
  Str(const char *s, Word n) {
    init(s, n);
  }
  Str(Word size) {
    init(size);
  }
  Str() {
    init(sizeof(Word) * 2 - 1);
  }
  Str(const Str *str) {
    init(str->_data, str->_len);
  }
  Str *clone() {
    return new Str(this);
  }
  void expand(Word newlen) {
    if (newlen > _cap)
      resize(nextPow2(newlen));
  }
  void shrink(bool fit = true) {
    if (_len != _cap)
      resize(_len);
  }
  Str *add(char ch) {
    expand(_len + 1);
    _data[_len++] = ch;
    _data[_len] = 0;
    return this;
  }
  Str *add(const char *s, Word n) {
    expand(_len + n);
    memcpy(_data + _len, s, n);
    _len += n;
    _data[_len] = 0;
    return this;
  }
  Str *add(Str *other) {
    return add(other->_data, other->_len);
  }
  Str *add(const char *s) {
    return add(s, strlen(s));
  }
  Str *substr(Word start, Word count);
  Str *chomp();
  Word find(Str *str, Word from = 0);
  Word find(char ch, Word from = 0);
  Word find(const char *s, Word from = 0);
  Word find(const char *s, Word n, Word from);
  Word rfind(Str *str);
  Word rfind(char ch);
  Word rfind(const char *s);
  Word rfind(const char *s, Word n);
  StrArr *split(Str *sep);
  StrArr *split(char ch);
  StrArr *split(const char *s);
  StrArr *split(const char *s, Word n);
  StrArr *splitLines();
  bool startsWith(Str *str);
  bool startsWith(const char *s);
  bool startsWith(const char *s, Word n);
  bool endsWith(Str *str);
  bool endsWith(const char *s);
  bool endsWith(const char *s, Word n);
  bool eq(Str *str);
  bool eq(const char *str);
  bool eq(const char *str, Word n);
  Str &operator<<(char ch) {
    add(ch);
    return *this;
  }
  Str &operator<<(Str *str) {
    add(str);
    return *this;
  }
  Str &operator<<(const char *str) {
    add(str);
    return *this;
  }
  Word len() {
    return _len;
  }
  Word count() {
    return _len;
  }
  char &ch(Word i) {
    require(i < _len, "string index out of range");
    return _data[i];
  }
  char &operator[](Word i) {
    require(i < _len, "string index out of range");
    return _data[i];
  }
  char *c_str() {
    return _data;
  }
  unsigned char *u_str() {
    return (unsigned char *) _data;
  }
};

Str *StrJoin(StrArr *arr, Str *sep);
Str *StrJoin(StrArr *arr, char ch);
Str *StrJoin(StrArr *arr, const char *sep);
Str *StrJoin(StrArr *arr, const char *sep, Word n);

static inline Str *S(const char *str) {
  return new Str(str);
}

#define StrArrLit(a) (new StrArr(NUMOF(a), a, CStrToStr))

int Cmp(Str *str1, Str *str2);
Str *ToStr(Int x);
Str *ToStr(Word x);
Str *CStrToStr(const char *s);
