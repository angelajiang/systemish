#include <thread>
#include "spdlog/spdlog.h"

namespace spd = spdlog;

static constexpr bool kVerbose = true;
#define erpc_dwarn(fmt, ...)           \
  do {                                 \
    if (kVerbose) {                    \
      console->warn(fmt, __VA_ARGS__); \
    }                                  \
  } while (0)

#define erpc_dinfo(fmt, ...)           \
  do {                                 \
    if (kVerbose) {                    \
      console->info(fmt, __VA_ARGS__); \
    }                                  \
  } while (0)

#define erpc_derror(fmt, ...)           \
  do {                                  \
    if (kVerbose) {                     \
      console->error(fmt, __VA_ARGS__); \
    }                                   \
  } while (0)

#define erpc_ddebug(fmt, ...)           \
  do {                                  \
    if (kVerbose) {                     \
      console->debug(fmt, __VA_ARGS__); \
    }                                   \
  } while (0)

void console_thread(std::shared_ptr<spd::logger> console) {
  // Customize msg format for all messages
  spd::set_pattern("[%H:%M:%S:%f thread %t] eRPC: %v");

  for (size_t i = 0; i < 100; i++) {
    erpc_derror("Some error message with arg{}..", 1);
    erpc_dwarn("Easy padding in numbers like {:08d}", 12);
    erpc_dinfo("Support for floats {:03.2f}", 1.23456);
    erpc_dinfo("Positional args are {1} {0}..", "too", "supported");
    erpc_dinfo("{:<30}", "left aligned");

    // Runtime log levels
    spd::set_level(spd::level::info);  // Set global log level to info
    erpc_ddebug("This message shold not be displayed! {0}", "foo");
    console->set_level(spd::level::debug);  // Set specific logger's log level
    erpc_ddebug("This message shold be displayed!, {0}", "bar");
  }
}

int main() {
  std::shared_ptr<spd::logger> console = spd::stdout_color_mt("console");

  auto thread_1 = std::thread(console_thread, console);
  auto thread_2 = std::thread(console_thread, console);

  thread_1.join();
  thread_2.join();
  
  spd::drop_all();
}
