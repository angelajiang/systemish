#include "json.hh"
#include "kvrow.hh"
#include "kvthread.hh"
#include "masstree.hh"
#include "masstree_insert.hh"
#include "masstree_tcursor.hh"
#include "query_masstree.hh"

class MtIndex {
 public:
  MtIndex() {}
  ~MtIndex() {}

  inline void setup(threadinfo *ti) {
    table_ = new Masstree::default_table();
    table_->initialize(*ti);
  }

  // Garbage Collection
  inline void clean_rcu(threadinfo *ti) { ti->rcu_quiesce(); }

  // Insert Unique
  inline bool put_uv(const Str &key, const Str &value, threadinfo *ti) {
    Masstree::default_table::cursor_type lp(table_->table(), key);
    bool found = lp.find_insert(*ti);
    if (!found) {
      ti->observe_phantoms(lp.node());
    } else {
      lp.finish(1, *ti);
      return false;
    }

    qtimes_.ts = ti->update_timestamp();
    qtimes_.prev_ts = 0;
    lp.value() = row_type::create1(value, qtimes_.ts, *ti);
    lp.finish(1, *ti);

    return true;
  }

  bool put_uv(const char *key, int keylen, const char *value, int valuelen,
              threadinfo *ti) {
    return put_uv(Str(key, keylen), Str(value, valuelen), ti);
  }

  // Upsert
  inline void put(const Str &key, const Str &value, threadinfo *ti) {
    Masstree::default_table::cursor_type lp(table_->table(), key);
    bool found = lp.find_insert(*ti);
    if (!found) {
      ti->observe_phantoms(lp.node());
      qtimes_.ts = ti->update_timestamp();
      qtimes_.prev_ts = 0;
    } else {
      qtimes_.ts = ti->update_timestamp(lp.value()->timestamp());
      qtimes_.prev_ts = lp.value()->timestamp();
      lp.value()->deallocate_rcu(*ti);
    }

    lp.value() = row_type::create1(value, qtimes_.ts, *ti);
    lp.finish(1, *ti);
  }

  void put(const char *key, int keylen, const char *value, int valuelen,
           threadinfo *ti) {
    put(Str(key, keylen), Str(value, valuelen), ti);
  }

  // Get (unique value)
  inline bool dynamic_get(const Str &key, Str &value, threadinfo *ti) {
    Masstree::default_table::unlocked_cursor_type lp(table_->table(), key);
    bool found = lp.find_unlocked(*ti);
    if (found) value = lp.value()->col(0);
    return found;
  }

  bool get(const char *key, int keylen, Str &value, threadinfo *ti) {
    return dynamic_get(Str(key, keylen), value, ti);
  }

  // Get Next (ordered)
  bool dynamic_get_next(Str &value, char *cur_key, int *cur_keylen,
                        threadinfo *ti) {
    lcdf::Json req = lcdf::Json::array(0, 0, Str(cur_key, *cur_keylen), 2);
    q_[0].run_scan(table_->table(), req, *ti);
    if (req.size() < 4) return false;
    value = req[3].as_s();
    if (req.size() < 6) {
      *cur_keylen = 0;
    } else {
      Str cur_key_str = req[4].as_s();
      memcpy(cur_key, cur_key_str.s, static_cast<size_t>(cur_key_str.len));
      *cur_keylen = cur_key_str.len;
    }

    return true;
  }

  // Get Next N (ordered)
  struct scanner {
    Str *values;
    int range;

    scanner(Str *values, int range) : values(values), range(range) {}

    template <typename SS2, typename K2>
    void visit_leaf(const SS2 &, const K2 &, threadinfo &) {}
    bool visit_value(Str, const row_type *row, threadinfo &) {
      *values = row->col(0);
      ++values;
      --range;
      return range > 0;
    }
  };

  int get_next_n(Str *values, char *cur_key, int *cur_keylen, int range,
                 threadinfo *ti) {
    if (range == 0) return 0;

    scanner s(values, range);
    int count = table_->table().scan(Str(cur_key, *cur_keylen), true, s, *ti);
    return count;
  }

 private:
  Masstree::default_table *table_;
  query<row_type> q_[1];
  loginfo::query_times qtimes_;
};
