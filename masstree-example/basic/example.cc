#include "json.hh"
#include "kvrow.hh"
#include "kvthread.hh"
#include "masstree.hh"
#include "masstree_tcursor.hh"
#include "query_masstree.hh"

volatile mrcu_epoch_type globalepoch = 1;
volatile mrcu_epoch_type active_epoch = 1;
volatile bool recovering = false;
kvtimestamp_t initial_timestamp;
kvepoch_t global_log_epoch = 0;

class mt_index {
 public:
  mt_index() {}
  ~mt_index() { ti_->rcu_stop(); }

  void setup() {
    ti_ = threadinfo::make(threadinfo::TI_MAIN, -1);
    ti_->rcu_start();
    table_ = new Masstree::default_table();
    table_->initialize(*ti_);
  }

  void put(const Masstree::Str &key, const Masstree::Str &value) {
    q_[0].run_replace(table_->table(), key, value, *ti_);
  }

  void put(const char *key, int keylen, const char *value, int valuelen) {
    put(Masstree::Str(key, keylen), Masstree::Str(value, valuelen));
  }

  bool get(const Masstree::Str &key, Masstree::Str &value) {
    return q_[0].run_get1(table_->table(), key, 0, value, *ti_);
  }

  bool get(const char *key, int keylen, Masstree::Str &value) {
    return q_[0].run_get1(table_->table(), Masstree::Str(key, keylen), 0, value,
                          *ti_);
  }

  bool remove(const char *key, int keylen) {
    return remove(Masstree::Str(key, keylen));
  }

  bool remove(const Masstree::Str &key) {
    return q_[0].run_remove(table_->table(), key, *ti_);
  }

 private:
  Masstree::default_table *table_;
  threadinfo *ti_;
  query<row_type> q_[1];
};

int main() {
  mt_index mti;
  mti.setup();

  std::cout << "\nPUT and GET test\n";
  mti.put("foo", 3, "bar", 3);
  mti.put("baz", 3, "bork", 4);

  Str value;
  bool get_success;
  get_success = mti.get("foo", 3, value);
  std::cout << get_success << "\t" << value.s << "\n";
  get_success = mti.get("bar", 3, value);
  std::cout << get_success << "\t" << value.s << "\n";
  get_success = mti.get("dave", 4, value);
  std::cout << get_success << "\t" << value.s << "\n";

  std::cout << "\nUPDATE test\n";
  mti.put("foo", 3, "barbar", 6);
  get_success = mti.get("foo", 3, value);
  std::cout << get_success << "\t" << value.s << "\n";

  std::cout << "\nREMOVE test\n";
  mti.remove("foo", 3);
  get_success = mti.get("foo", 3, value);
  std::cout << get_success << "\t" << value.s << "\n";

  return 0;
}
