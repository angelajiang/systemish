#include <thread>
#include "mt_index_api.h"

static constexpr size_t kNumWorkerThreads = 2;  // Workers, excluding main

void thread_func(MtIndex *mti, threadinfo *ti, size_t worker_id) {
  for (size_t i = 0; i < 1000000; i++) {
    if (worker_id == 0) {
      mti->put("foo", 3, "worker_0", 8, ti);
      mti->put("bar", 3, "worker_0", 8, ti);
    } else {
      mti->put("foo", 3, "worker_1", 8, ti);
      mti->put("bar", 3, "worker_1", 8, ti);
    }

    Str value;
    bool get_success = mti->get("foo", 3, value, ti);
    if (!get_success) throw std::runtime_error("Get failed");
    if (std::string(value.s) != "worker_0" &&
        std::string(value.s) != "worker_1") {
      throw std::runtime_error("Invald foo value " + std::string(value.s));
    }

    get_success = mti->get("bar", 3, value, ti);
    if (!get_success) throw std::runtime_error("Get failed");
    if (std::string(value.s) != "worker_0" &&
        std::string(value.s) != "worker_1") {
      throw std::runtime_error("Invald foo value " + std::string(value.s));
    }
  }
}

int main() {
  // Create the Masstree using the main thread
  threadinfo *ti = threadinfo::make(threadinfo::TI_MAIN, -1);
  MtIndex mti;
  mti.setup(ti);

  // Create threadinfo structs for worker threads
  std::vector<threadinfo *> ti_vec;
  for (size_t i = 0; i < kNumWorkerThreads; i++) {
    ti_vec.push_back(threadinfo::make(threadinfo::TI_PROCESS, i));
  }

  std::thread threads[kNumWorkerThreads];
  for (size_t i = 0; i < kNumWorkerThreads; i++) {
    threads[i] = std::thread(thread_func, &mti, ti_vec[i], i);
  }

  for (size_t i = 0; i < kNumWorkerThreads; i++) threads[i].join();
  return 0;
}
