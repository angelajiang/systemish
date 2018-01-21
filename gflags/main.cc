#include <gflags/gflags.h>
#include <iostream>

DEFINE_bool(verbose, false, "Display program name before message");
DEFINE_string(message, "", "Message to print");
DEFINE_uint64(size, 0, "Data size");

int main(int argc, char *argv[]) {
  gflags::SetUsageMessage("Usage");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_verbose) std::cout << gflags::ProgramInvocationShortName() << ": ";
  printf("message length = %zu, message = |%s|\n", FLAGS_message.size(), FLAGS_message.c_str());
  std::cout << FLAGS_message << std::endl;
  std::cout << "Data size " << FLAGS_size << std::endl;
  gflags::ShutDownCommandLineFlags();
  return 0;
}
