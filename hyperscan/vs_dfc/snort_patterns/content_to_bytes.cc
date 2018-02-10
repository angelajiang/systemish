// Process a content_strings.txt file to generate byte lists
// The output should be redirected to content_bytes.txt
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>

enum class Mode { kChar, kHex };

Mode invert(Mode m) {
  if (m == Mode::kChar) return Mode::kHex;
  return Mode::kChar;
}

bool is_hex_char(char c) { return (c >= 'A' && c <= 'F'); }

bool is_hex_number(char c) { return (c >= '0' && c <= '9'); }

bool is_hex(char c) { return is_hex_char(c) || is_hex_number(c); }

size_t hex_to_int(char c) {
  assert(is_hex(c));
  if (is_hex_number(c)) return static_cast<size_t>(c - '0');
  if (is_hex_char(c)) return (10 + static_cast<size_t>(c - 'A'));
  return SIZE_MAX;
}

int main() {
  std::ifstream virus_file("content_strings.txt");

  while (true) {
    std::string line;

    std::getline(virus_file, line);
    if (line.empty()) break;

    std::string ret;
    auto mode = Mode::kChar;
    for (size_t i = 0; i < line.length(); i++) {
      char c = line.at(i);
      if (c == '|') {
        mode = invert(mode);
        continue;
      }

      if (mode == Mode::kHex && c == ' ') continue;

      if (mode == Mode::kChar) {
        // Process one character
        ret += std::to_string(static_cast<size_t>(c)) + " ";
      } else {
        // Process two characters
        assert(is_hex(c));
        assert(i + 1 < line.length());
        i++;
        char c2 = line.at(i);

        size_t number = hex_to_int(c2) + (16 * hex_to_int(c));
        ret += std::to_string(number) + " ";
      }
    }

    assert(mode == Mode::kChar);
    printf("%s\n", ret.c_str());
  }
}
