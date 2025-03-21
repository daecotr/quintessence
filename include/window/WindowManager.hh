#pragma once

#include "common/Singleton.hh"

namespace Q {

class WindowManager : public Singleton<WindowManager> {
  friend class Singleton<WindowManager>;

protected:
  WindowManager();
  ~WindowManager();
};

} // namespace Q