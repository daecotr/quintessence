#include "common/TerminalLogger.hh"

#include "common/Config.hh"

#include <iostream>
#include <sstream>

namespace Q {

void TerminalLogger::write(const LogLevel logLevel, const char *message) {
  std::ostream *stream = &std::cout;
  std::string prefix;
  switch (logLevel) {
  case LogLevel::info:
    prefix = "[information] ";
    break;
  case LogLevel::debug:
    if constexpr (!DEBUG_MODE) {
      stream = nullptr;
      break;
    } else {
      prefix = "\033[33m[debug]\033[0m ";
      break;
    }
  case LogLevel::warn:
    prefix = "\033[35m[warning]\033[0m ";
    break;
  case LogLevel::error:
    prefix = "\033[31m[error]\033[0m ";
    stream = &std::cerr;
    break;
  }

  // ReSharper disable once CppDFAConstantConditions, `stream = nullptr` when
  // `!DEBUG_MODE`
  if (stream)
    (*stream) << prefix.c_str() << message << std::endl;

  // Doesn't need to delete pointer to global object
}

} // namespace Q