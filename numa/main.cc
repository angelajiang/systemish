#include <stdio.h>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <assert.h>

int main() {
  std::ifstream infile("/proc/cpuinfo");
  std::map<size_t, size_t> lcore_to_numa_map;

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

      int file_socket = std::atoi(split_vec.at(1).c_str());
      assert(file_socket >= 0 && file_socket <= 4);  // 4 sockets

      lcore_to_numa_map[cur_lcore] = static_cast<size_t>(file_socket);
      cur_lcore++;
    }
  }

  for (auto p : lcore_to_numa_map) {
    printf("lcore %zu, numa %zu\n", p.first, p.second);
  }

  return 0;
}
