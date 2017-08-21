#include <thread>
#include "mt_index_api.h"

static constexpr size_t kNumWorkerThreads = 2;  // Workers, excluding main
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
    mti->put(1, worker_id, ti);
    mti->put(2, worker_id, ti);

    size_t value;
    bool get_success = mti->get(1, value, ti);
    if (!get_success) throw std::runtime_error("Get(1) failed");
    if (value >= kNumWorkerThreads) {
      throw std::runtime_error("Invald value " + std::to_string(value));
    }

    get_success = mti->get(2, value, ti);
    if (!get_success) throw std::runtime_error("Get(2) failed");
    if (value >= kNumWorkerThreads) {
      throw std::runtime_error("Invald value " + std::to_string(value));
    }
  }
}

// Test the performance of point GETs
void get_perf_func(MtIndex *mti, threadinfo *ti, size_t worker_id) {
  FastRand fast_rand(worker_id);

  struct timespec start, end;

  while (true) {
    clock_gettime(CLOCK_REALTIME, &start);

    for (size_t i = 0; i < 1000000; i++) {
      size_t rand_key = fast_rand.next_u32() % kNumKeys;
      size_t value;

      bool success = mti->get(rand_key, value, ti);

      if (!success) {
        throw std::runtime_error("Get failed for key " +
                                 std::to_string(rand_key));
      }

      if (value != rand_key) {
        throw std::runtime_error("Incorrect value " + std::to_string(value) +
                                 " for key" + std::to_string(rand_key));
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
    mti.put(key, value, ti);
  }

  // Do some scans
  for (size_t iter = 0; iter < 10; iter++) {
    struct timespec scan_start, scan_end;
    clock_gettime(CLOCK_REALTIME, &scan_start);

    size_t start_key = kNumKeys / 2;
    size_t count = mti.count_in_range(start_key, kNumKeys, ti);

    clock_gettime(CLOCK_REALTIME, &scan_end);

    double seconds = (scan_end.tv_sec - scan_start.tv_sec) +
                     (scan_end.tv_nsec - scan_start.tv_nsec) / 1000000000.0;

    printf("Time to scan %zu keys = %.2f usec.\n", count, seconds * 1000000);
  }

  // Create threadinfo structs for worker threads
  std::vector<threadinfo *> ti_vec;
  for (size_t i = 0; i < kNumWorkerThreads; i++) {
    ti_vec.push_back(threadinfo::make(threadinfo::TI_PROCESS, i));
  }

  std::thread threads[kNumWorkerThreads];
  for (size_t i = 0; i < kNumWorkerThreads; i++) {
    threads[i] = std::thread(get_perf_func, &mti, ti_vec[i], i);
  }

  for (size_t i = 0; i < kNumWorkerThreads; i++) threads[i].join();
  return 0;
}
