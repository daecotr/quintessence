#pragma once

#include "ILogger.hh"

namespace Q {

class TerminalLogger final : public ILogger {
public:
  void write(LogLevel logLevel, const char *message) override;
};

} // namespace Q
