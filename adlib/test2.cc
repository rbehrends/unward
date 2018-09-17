#include "lib.h"

void Main() {
  StrArr *arr
      = ReadProcessLines(new Str("seq"), new StrArr(1, new Str("10000")));
  Check(arr->len() == 10001, "read process output as lines");
  Word hash = 0;
  for (StrArr::Each it(arr); it; ++it) {
    hash += Hash(*it);
  }
  Check((hash & 0xffff) == 0xc756, "hashing strings");
  Str *data = ReadFile("/dev/null");
  Check(data && data->len() == 0, "read bytes from file");
  arr = ReadLines(__FILE__);
  Check(arr && arr->len() > __LINE__, "read lines from file");
  Check(WriteFile("/dev/null", S("test")), "write bytes to file");
  static CString args[] = { "17" };
  Check(System(S("exit"), StrArrLit(args)) == 17, "invoke shell");
}
