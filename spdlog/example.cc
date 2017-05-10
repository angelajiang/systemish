//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
//
// spdlog usage example
//
//
#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>

namespace spd = spdlog;
int main(int, char* []) {
  // Console logger with color
  auto console = spd::stdout_color_mt("console");
  console->info("Welcome to spdlog!");
  console->error("Some error message with arg{}..", 1);

  // Formatting examples
  console->warn("Easy padding in numbers like {:08d}", 12);
  console->critical(
      "Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
  console->info("Support for floats {:03.2f}", 1.23456);
  console->info("Positional args are {1} {0}..", "too", "supported");
  console->info("{:<30}", "left aligned");

  spd::get("console")->info(
      "loggers can be retrieved from a global registry using the "
      "spdlog::get(logger_name) function");

  // Customize msg format for all messages
  spd::set_pattern("*** [%H:%M:%S %z] [thread %t] %v ***");

  // Runtime log levels
  spd::set_level(spd::level::info);  // Set global log level to info
  console->debug("This message shold not be displayed!");
  console->set_level(spd::level::debug);  // Set specific logger's log level
  console->debug("This message shold be displayed..");

  // Compile time log levels
  // define SPDLOG_DEBUG_ON or SPDLOG_TRACE_ON
  SPDLOG_TRACE(console, "Enabled only #ifdef SPDLOG_TRACE_ON..{} ,{}", 1, 3.23);
  SPDLOG_DEBUG(console, "Enabled only #ifdef SPDLOG_DEBUG_ON.. {} ,{}", 1,
               3.23);

  // Release and close all loggers
  spd::drop_all();
}
