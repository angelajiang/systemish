#include <assert.h>
#include <stdio.h>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <map>
#include <string>
#include <vector>

int main() {
  std::ifstream infile("/proc/cpuinfo");
  std::vector<size_t> numa_lcores[4];

  std::string line;
  std::vector<std::string> split_vec;

  size_t cur_lcore = 0;
  while (std::getline(infile, line)) {
    if (line.find("processor") != std::string::npos) {
      boost::split(split_vec, line, boost::is_any_of(":"));
      boost::trim(split_vec.at(1));

      int file_lcore = std::atoi(split_vec.at(1).c_str());
      assert(file_lcore == static_cast<int>(cur_lcore));
    }

    if (line.find("physical id") != std::string::npos) {
      boost::split(split_vec, line, boost::is_any_of(":"));
      boost::trim(split_vec.at(1));

      int numa = std::atoi(split_vec.at(1).c_str());
      assert(numa >= 0 && numa <= 3);  // Up to 4 sockets

      numa_lcores[numa].push_back(cur_lcore);
      cur_lcore++;
    }
  }

  for (size_t i = 0; i < 4; i++) {
    printf("NUMA %zu\n", i);
    if (numa_lcores[i].empty()) continue;

    for (size_t lcore : numa_lcores[i]) printf("%zu ", lcore);
    printf("\n");
  }

  return 0;
}
