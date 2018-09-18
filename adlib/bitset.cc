#include "lib.h"
#include "bitset.h"

#define MEMSIZE(n) ((n+31) >> 5)

static inline Word bitcount(Word32 word) {
  word = word - ((word >> 1) & 0x55555555);
  word = (word & 0x33333333) + ((word >> 2) & 0x33333333);
  return (((word + (word >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

void BitSet::init(Word n) {
  _bits = n;
  _words = MEMSIZE(n);
  _data = (Word32 *) GC_MALLOC_ATOMIC(sizeof(Word32) * _words);
  memset(_data, 0, sizeof(Word32) * _words);
}

void BitSet::zero() {
  memset(_data, 0, sizeof(Word32) * _words);
}

void BitSet::resize(Word n) {
  Word words = _words;
  Word32 *data = _data;
  _bits = n;
  _words = MEMSIZE(n);
  _data = (Word32 *) GC_MALLOC_ATOMIC(sizeof(Word32) * _words);
  memcpy(_data, data, sizeof(Word32) * Min(words, _words));
  if (_words > words) {
    memset(_data + words, 0, (_words - words) * sizeof(Word32));
  }
}
void BitSet::expand(Word n) {
  if (n > _bits)
    resize(n);
}

Word BitSet::count() {
  Word result = 0;
  for (Word i = 0; i < _words; i++) {
    if (_data[i])
      result += bitcount(_data[i]);
  }
  return result;
}

BitSet *BitSet::complement_in_place() {
  for (Word i = 0; i < _words; i++) {
    _data[i] = ~_data[i];
  }
  Word n = _bits & 32;
  if (n != 0) {
    // zero top bits
    _data[_words-1] &= ((1 << n) - 1);
  }
  return this;
}

BitSet *BitSet::complement() {
  return clone()->complement_in_place();
}

BitSet *BitSet::union_in_place(BitSet *that) {
  expand(that->_bits);
  for (Word i = 0; i < _words; i++)
    _data[i] |= that->_data[i];
  return this;
}

BitSet * BitSet::union_with(BitSet *that) {
  BitSet *result = clone();
  result->union_in_place(that);
  return result;
}

BitSet *BitSet::intersect_in_place(BitSet *that) {
  expand(that->_bits);
  for (Word i = 0; i < _words; i++)
    _data[i] &= that->_data[i];
  return this;
}

BitSet * BitSet::intersect_with(BitSet *that) {
  return clone()->intersect_in_place(that);
}

BitSet *BitSet::diff_in_place(BitSet *that) {
  expand(that->_bits);
  for (Word i = 0; i < _words; i++)
    _data[i] &= ~that->_data[i];
  return this;
}

BitSet * BitSet::diff_with(BitSet *that) {
  return clone()->diff_in_place(that);
}

BitMatrix *MakeBitMatrix(Word rows, Word cols) {
  BitMatrix *result = new BitMatrix();
  for (Word i = 0; i < rows; i++)
    result->add(new BitSet(cols));
  return result;
}

BitMatrix *Clone(BitMatrix *mat) {
  BitMatrix *result = new BitMatrix(mat->len());
  for (Word i = 0; i < mat->len(); i++)
    result->add(mat->at(i)->clone());
  return result;
}

bool IsMatrix(BitMatrix *mat) {
  if (mat->len() == 0 || mat->at(0)->len() == 0)
    return false;
  for (Word i = 1; i < mat->len(); i++) {
    if (mat->at(i-1)->len() != mat->at(i)->len()) {
      return false;
    }
  }
  return true;
}

BitMatrix *Transpose(BitMatrix *mat) {
  require(IsMatrix(mat), "not a proper matrix");
  Word rows = mat->len();
  Word cols = mat->at(0)->len();
  BitMatrix *result = MakeBitMatrix(cols, rows);
  for (Word col = 0; col < cols; col++) {
    BitSet *result_row = result->at(col);
    for (Word row = 0; row < rows; row++) {
      if (mat->at(row)->test(col)) {
        result_row->set(row);
      }
    }
  }
  return result;
}

BitMatrix *TransitiveClosure(BitMatrix *mat) {
  require(IsMatrix(mat), "not a proper matrix");
  Word rows = mat->len();
  Word cols = mat->at(0)->len();
  require(mat->len() == mat->at(0)->len(), "not a square matrix");
  mat = Clone(mat);
  for (Word col = 0; col < cols; col++) {
    for (Word row = 0; row < rows; row++) {
      if (mat->at(row)->test(col)) {
        mat->at(row)->union_in_place(mat->at(col));
      }
    }
  }
  return mat;
}