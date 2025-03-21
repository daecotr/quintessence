#include "common/LogManager.hh"

namespace Q {

void LogManager::write(const LogLevel logLevel, const char *message) {
  for (const auto &logger : loggers)
    logger->write(logLevel, message);
}

void LogManager::write(const LogLevel logLevel, const std::string &message) {
  write(logLevel, message.c_str());
}

} // namespace Q