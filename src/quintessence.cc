#include <cstdlib>

#include "common/LogManager.hh"
#include "common/TerminalLogger.hh"

int main(int argc, char **argv) {
  Q::LogManager::getInstance().attach<Q::TerminalLogger>();
  Q::LogManager::getInstance().write(Q::LogLevel::info, "Hello, this is information");
  Q::LogManager::getInstance().write(Q::LogLevel::debug, "Hello, this is debug");
  Q::LogManager::getInstance().write(Q::LogLevel::warn, "Hello, this is warning");
  Q::LogManager::getInstance().write(Q::LogLevel::error, "Hello, this is error");
  return EXIT_SUCCESS;
}