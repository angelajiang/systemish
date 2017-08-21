#include "json.hh"
#include "kvrow.hh"
#include "kvthread.hh"
#include "masstree.hh"
#include "masstree_insert.hh"
#include "masstree_scan.hh"
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

  struct scanner_t {
    int range;

    scanner_t(int range) : range(range) {}

    template <typename SS2, typename K2>
    void visit_leaf(const SS2 &, const K2 &, threadinfo &) {}

    bool visit_value(Str key, const row_type *row, threadinfo &) {
      Str value = row->col(0);
      if (value.len == 0) {
        printf("Value len = 0.\n");
      } else {
        printf("Visiting key %p, value %zu.\n",
               reinterpret_cast<size_t *>(
                   *reinterpret_cast<const size_t *>(key.s)),
               *reinterpret_cast<const size_t *>(value.s));
      }

      --range;
      return range > 0;
    }
  };

  int count_in_range(const char *cur_key, int cur_keylen, int range,
                     threadinfo *ti) {
    if (range == 0) return 0;

    scanner_t scanner(range);
    int count =
        table_->table().scan(Str(cur_key, cur_keylen), true, scanner, *ti);

    return count;
  }

 private:
  Masstree::default_table *table_;
  query<row_type> q_[1];
  loginfo::query_times qtimes_;
};
