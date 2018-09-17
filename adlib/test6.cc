#include "adlib/lib.h"
#include "adlib/bitset.h"

void Main() {
  BitSet *set = new BitSet(256);
  BitSet *set2 = new BitSet(256);
  for (Word i = 0; i < 200; i += 2) {
    set->set(i);
    set2->set(i+1);
  }
  for (Word i = 0; i < 200; i += 4) {
    set2->set(i);
    set2->clear(i+1);
  }
  Check(set->count() == 100, "bitset count");
  Check(set->union_with(set2)->count() == 150, "bitset union");
  Check(set->intersect_with(set2)->count() == 50, "bitset intersection");
  Check(set->diff_with(set2)->count() == 50, "bitset difference");
  Word sum = 0;
  for (BitSet::Each it(set); it; it++) {
    sum += *it;
  }
  Check(sum == 99 * 100, "bitset iteration");
}

