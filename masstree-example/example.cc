#include "json.hh"
#include "kvrow.hh"
#include "kvthread.hh"
#include "masstree.hh"
#include "query_masstree.hh"

volatile mrcu_epoch_type globalepoch = 1;
volatile mrcu_epoch_type active_epoch = 1;
volatile bool recovering = false;
kvtimestamp_t initial_timestamp;
kvepoch_t global_log_epoch = 0;

template <typename T>
class mt_index {
 public:
  mt_index() {}
  ~mt_index() { ti_->rcu_stop(); }

  void setup() {
    // ti_ = new threadinfo();
    // ti_->rcu_start();
    ti_ = threadinfo::make(threadinfo::TI_MAIN, -1);
    table_ = new T;
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

  bool get_next(const Masstree::Str &cur_key, Masstree::Str &key,
                Masstree::Str &value) {
    std::vector<Masstree::Str> keys;
    std::vector<Masstree::Str> values;
    lcdf::Json req = lcdf::Json::array(0, 0, cur_key, 2);
    q_[0].run_scan(table_->table(), req, *ti_);
    // output_scan(req, keys, values);
    keys.clear();
    values.clear();

    for (int i = 2; i != req.size(); i += 2) {
      keys.push_back(req[i].as_s());
      values.push_back(req[i + 1].as_s());
    }

    if ((keys.size() < 2) || (values.size() < 2)) return false;
    key = keys[1];
    value = values[1];
    return true;
  }

  bool get_next(const char *cur_key, int cur_keylen, Masstree::Str &key,
                Masstree::Str &value) {
    std::vector<Masstree::Str> keys;
    std::vector<Masstree::Str> values;
    lcdf::Json req =
        lcdf::Json::array(0, 0, Masstree::Str(cur_key, cur_keylen), 2);
    q_[0].run_scan(table_->table(), req, *ti_);
    // output_scan(req, keys, values);
    keys.clear();
    values.clear();

    for (int i = 2; i != req.size(); i += 2) {
      keys.push_back(req[i].as_s());
      values.push_back(req[i + 1].as_s());
    }

    if ((keys.size() < 2) || (values.size() < 2)) return false;

    key = keys[1];
    value = values[1];
    return true;
  }

  bool remove(const char *key, int keylen) {
    return remove(Masstree::Str(key, keylen));
  }

  bool remove(const Masstree::Str &key) {
    return q_[0].run_remove(table_->table(), key, *ti_);
  }

 private:
  T *table_;
  threadinfo *ti_;
  query<row_type> q_[1];
};

int main() {
  mt_index<Masstree::default_table> mti;
  mti.setup();
}
