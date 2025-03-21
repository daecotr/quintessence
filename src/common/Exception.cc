#include "common/Exception.hh"

namespace Q {

const char *Exception::what() const noexcept { return message.c_str(); }

} // namespace Q