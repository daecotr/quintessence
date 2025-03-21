#pragma once

namespace Q {

enum class LogLevel { info, debug, warn, error };

class ILogger {
public:
  virtual ~ILogger() = default;
  virtual void write(LogLevel logLevel, const char *message) = 0;
};

} // namespace Q
