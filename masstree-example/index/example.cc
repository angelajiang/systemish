#include <thread>
#include "mt_index_api.h"

static constexpr size_t kNumWorkerThreads = 1;  // Workers, excluding main
static constexpr size_t kNumKeys = (1024 * 1024);

class FastRand {
 public:
  uint64_t seed;
  FastRand(uint64_t seed) : seed(seed) {}

  inline uint32_t next_u32() {
    seed = seed * 1103515245 + 12345;
    return static_cast<uint32_t>(seed >> 32);
  }
};

// A worker thread function that does a simple concurrency test
void concurrency_test_func(MtIndex *mti, threadinfo *ti, size_t worker_id) {
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

// Test the performance of point GETs
void get_perf_test_func(MtIndex *mti, threadinfo *ti, size_t worker_id) {
  FastRand fast_rand(worker_id);

  struct timespec start, end;

  while (true) {
    clock_gettime(CLOCK_REALTIME, &start);

    for (size_t i = 0; i < 1000000; i++) {
      size_t rand_key = fast_rand.next_u32() % kNumKeys;

      Str value;
      bool success = mti->get(reinterpret_cast<const char *>(&rand_key),
                              sizeof(size_t), value, ti);

      if (!success || value.length() != sizeof(size_t)) {
        throw std::runtime_error("Get failed for key " +
                                 std::to_string(rand_key));
      }

      if (*reinterpret_cast<const size_t *>(value.s) != rand_key) {
        throw std::runtime_error("Incorrect value " + std::to_string(i) +
                                 " for" + std::string(value.s));
      }
    }

    clock_gettime(CLOCK_REALTIME, &end);
    double seconds = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Worker %zu: Tput = %.2f M GETs/s.\n", worker_id,
           1000000 / (seconds * 1000000));
  }
}

int main() {
  // Create the Masstree using the main thread
  threadinfo *ti = threadinfo::make(threadinfo::TI_MAIN, -1);
  MtIndex mti;
  mti.setup(ti);

  // Insert a million keys
  for (size_t i = 0; i < kNumKeys; i++) {
    size_t key = i;
    size_t value = i;
    mti.put(reinterpret_cast<const char *>(&key), sizeof(size_t),
            reinterpret_cast<const char *>(&value), sizeof(size_t), ti);
  }

  // Create threadinfo structs for worker threads
  std::vector<threadinfo *> ti_vec;
  for (size_t i = 0; i < kNumWorkerThreads; i++) {
    ti_vec.push_back(threadinfo::make(threadinfo::TI_PROCESS, i));
  }

  std::thread threads[kNumWorkerThreads];
  for (size_t i = 0; i < kNumWorkerThreads; i++) {
    threads[i] = std::thread(get_perf_test_func, &mti, ti_vec[i], i);
  }

  for (size_t i = 0; i < kNumWorkerThreads; i++) threads[i].join();
  return 0;
}
