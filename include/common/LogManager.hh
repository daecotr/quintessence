#pragma once

#include <vector>

#include "common/ILogger.hh"
#include "common/Singleton.hh"

namespace Q {

class LogManager : public Singleton<LogManager> {
public:
  friend class Singleton<LogManager>;

  /**
   * @brief Creates & attach new logger.
   * @tparam TLogger Logger class inherits from `ILogger`.
   * @tparam Args `args` type.
   * @param args Params of `TLogger` construct.
   */
  template <typename TLogger, typename... Args>
    requires std::is_base_of_v<ILogger, TLogger>
  void attach(Args &&...args);

  void write(LogLevel logLevel, const char *message);
  void write(LogLevel logLevel, const std::string &message);

private:
  std::vector<std::unique_ptr<ILogger>> loggers;
};

template <typename TLogger, typename... Args>
  requires std::is_base_of_v<ILogger, TLogger>
void LogManager::attach(Args &&...args) {
  loggers.push_back(
      std::unique_ptr<ILogger>(new TLogger(std::forward<Args>(args)...)));
}

} // namespace Q
