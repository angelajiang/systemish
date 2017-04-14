#include <stdio.h>
#include <string>
#include <unordered_map>

int main() {
  std::unordered_map<std::string, size_t> map;

  char A[100];
  sprintf(A, "asd");
  map[std::string(A)] = 5;

  map["asd"] = 5;
  map.insert(std::pair<std::string, size_t>("asd", 2));
  map.insert(std::pair<std::string, size_t>("asd", 3));
  map.insert(std::pair<std::string, size_t>("asd", 4));

  printf("%zu\n", map.count("asd"));
  printf("%zu\n", map["asd"]);
}
