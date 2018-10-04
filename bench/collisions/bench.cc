#include <city.h>
#include <stdio.h>
#include <stdlib.h>
#include <pcg/pcg_random.hpp>
#include <random>
#include <vector>

static constexpr size_t kBucketCap = 8;
static constexpr size_t kNumKeys = 1024 * 1024 * 32;
static constexpr size_t kNumBuckets = 1.5 * (kNumKeys / kBucketCap);

int main() {
  pcg64_fast pcg(pcg_extras::seed_seq_from<std::random_device>{});
  std::vector<size_t> buckets;
  buckets.resize(kNumBuckets);
  for (size_t &b : buckets) b = 0;

  for (size_t i = 0; i < kNumKeys; i++) {
    size_t rand_value;

    // Pick a method of creating the rand_value
    rand_value = CityHash64(reinterpret_cast<const char *>(&i), sizeof(size_t));
    rand_value = pcg();
    buckets[pcg() % kNumBuckets]++;
  }

  std::vector<size_t> hist(kBucketCap * 3);
  for (auto &h : hist) h = 0;

  size_t num_buckets_required = 0;
  for (size_t i = 0; i < kNumBuckets; i++) {
    if (buckets[i] < hist.size()) hist[buckets[i]]++;
    if (buckets[i] < kBucketCap) {
      num_buckets_required += 1;
      continue;
    }

    num_buckets_required += (buckets[i] + kBucketCap - 1) / kBucketCap;
  }

  printf("extra_buckets / regular_buckets = %.2f, table occupancy = %.2f\n",
         (num_buckets_required - kNumBuckets) * 1.0 / kNumBuckets,
         kNumKeys * 1.0 / (num_buckets_required * kBucketCap));

  for (size_t i = 0; i < hist.size(); i++) {
    printf("Fraction of buckets with %zu items = %.2f\n", i,
           hist[i] * 1.0 / kNumBuckets);
  }
}
