#include <cstdlib>

#include "common/LogManager.hh"
#include "common/TerminalLogger.hh"
#include "window/Window.hh"

int main(int argc, char **argv) {
  Q::LogManager::getInstance().attach<Q::TerminalLogger>();
  Q::Window window{512, 512};
  return EXIT_SUCCESS;
}