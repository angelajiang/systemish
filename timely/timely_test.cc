#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>

static double kTimelyMaxRate = 5.0 * 1000 * 1000 * 1000;
static double kTimelyMinRate = 5.0 * 1000 * 1000;
static double kTimelyAddRate = 5.0 * 1000 * 1000;
static double kMinRTT = 2;
static double kTLow = 50;
static double kTHigh = 1000;
static double kEwmaAlpha = .875;
static double kBeta = .008;

class Timely {
 public:
  double prev_rtt = kMinRTT;
  double avg_rtt_diff = 0.0;
  double rate = kTimelyMaxRate;

  Timely() {}

  static double w(double g) {
    if (g <= -0.25) return 0;
    if (g >= 0.25) return 1;
    return (2 * g + 0.5);
  }

  void update_rate(double sample_rtt) {
    assert(sample_rtt > kTLow);
    double new_rate;

    if (sample_rtt <= kTHigh) {
      double rtt_diff = sample_rtt - prev_rtt;
      prev_rtt = sample_rtt;
      avg_rtt_diff =
          ((1 - kEwmaAlpha) * avg_rtt_diff) + (kEwmaAlpha * rtt_diff);

      double norm_grad = avg_rtt_diff / kMinRTT;
      double weight = w(norm_grad);
      double error = (sample_rtt - kTLow) / kTLow;
      new_rate =
          rate * (1 - kBeta * weight * error) + kTimelyAddRate * (1 - weight);
    } else {
      new_rate = rate * (1 - kBeta * (1 - kTHigh / sample_rtt));
    }

    rate = std::max(new_rate, rate * 0.5);
    rate = std::min(rate, kTimelyMaxRate);
    rate = std::max(rate, kTimelyMinRate);
  }

  /// Convert a default bytes/second rate to Gbit/s
  static double rate_to_gbps(double r) {
    return (r / (1000 * 1000 * 1000)) * 8;
  }
};

void test(size_t rtt) {
  double P = kTimelyAddRate * kTLow / (kBeta * (rtt - kTLow));  // Predict

  Timely timely;
  for (size_t i = 0; i < 20000; i++) {
    timely.update_rate(rtt + rand() % 40);
  }
  printf("RTT %zu us, steady tput %.2f Gbps, prediction = %.2f\n", rtt,
         Timely::rate_to_gbps(timely.rate), Timely::rate_to_gbps(P));
}

int main() {
  for (size_t iter = 1; iter <= 20; iter++) test(kTLow + iter * 20);
}
