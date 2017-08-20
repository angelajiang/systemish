#include "mt_index_api.h"

int main() {
  threadinfo *ti = threadinfo::make(threadinfo::TI_MAIN, -1);
  MtIndex mti;
  mti.setup(ti);

  std::cout << "\nPUT and GET test\n";
  mti.put("foo", 3, "bar", 3, ti);
  mti.put("baz", 3, "bork", 4, ti);

  Str value;
  bool get_success;
  get_success = mti.get("foo", 3, value, ti);
  std::cout << get_success << "\t" << value.s << "\n";
  get_success = mti.get("baz", 3, value, ti);
  std::cout << get_success << "\t" << value.s << "\n";
  get_success = mti.get("dave", 4, value, ti);
  std::cout << get_success << "\t" << value.s << "\n";

  std::cout << "\nUPDATE test\n";
  mti.put("foo", 3, "barbar", 6, ti);
  get_success = mti.get("foo", 3, value, ti);
  std::cout << get_success << "\t" << value.s << "\n";

  return 0;
}
