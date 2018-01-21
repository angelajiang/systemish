#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

const double kTimelyMaxRate = 5.0 * 1000 * 1000 * 1000;  // 40 Gbps
const double kTimelyMinRate = 5.0 * 1000 * 1000;
const double kTimelyAddRate = 5.0 * 1000 * 1000;

class Timely {
 public:
  const double kMinRTT = 2;
  const double kTLow = 50;
  const double kEwmaAlpha = .875;
  const double kBeta = .008;

  double prev_rtt = kMinRTT;
  double avg_rtt_diff = 0.0;
  double rate = kTimelyMaxRate;

  Timely() {}

  static double w_func(double g) {
    if (g <= -0.25) return 0;
    if (g >= 0.25) return 1;
    return (2 * g + 0.5);
  }

  void update_rate(double sample_rtt) {
    double rtt_diff = sample_rtt - prev_rtt;
    avg_rtt_diff = ((1 - kEwmaAlpha) * avg_rtt_diff) + (kEwmaAlpha * rtt_diff);

    double new_rate;
    if (sample_rtt < kTLow) {
      new_rate = rate + kTimelyAddRate;
    } else {
      double norm_grad = avg_rtt_diff / kMinRTT;
      double wght = w_func(norm_grad);
      double err = (sample_rtt - kTLow) / kTLow;
      new_rate = rate * (1 - kBeta * wght * err) + kTimelyAddRate * (1 - wght);
    }

    rate = std::max(new_rate, rate * 0.5);
    rate = std::min(rate, kTimelyMaxRate);
    rate = std::max(rate, kTimelyMinRate);

    prev_rtt = sample_rtt;
  }

  /// Convert a default bytes/second rate to Gbit/s
  static double rate_to_gbps(double r) {
    return (r / (1000 * 1000 * 1000)) * 8;
  }
};

void test(size_t rtt) {
  Timely timely;
  for (size_t i = 0; i < 2000; i++) timely.update_rate(rtt);

  printf("mean %zu us, steady tput %.2f Gbps\n", rtt,
         Timely::rate_to_gbps(timely.rate));
}

int main() {
  for (size_t iter = 0; iter < 20; iter++) test(5 + iter * 10);
}
