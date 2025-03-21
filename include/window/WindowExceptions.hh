#pragma once

#include "common/Exception.hh"

namespace Q {

class WindowException : public Exception {
public:
  using Exception::Exception;
};

class WindowCreationFailedException final : public WindowException {
public:
  using WindowException::WindowException;
};

} // namespace Q