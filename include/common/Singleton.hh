#pragma once

#include <memory>

namespace Q {

/**
 * @brief Singleton pattern implementation base class.
 * This class provides the foundation for implementing the Singleton design pattern.
 * A derived class `TClass` inherits from `Singleton<TClass>` to ensure only a single instance exists.
 *
 * @details
 * Derive your class from `Singleton<TClass>`.
 * Declare `friend class Singleton<TClass>` in the derived class.
 * Make the constructor(s) and destructor of the derived class protected
 * access.
 * Use `getInstance()` to access the singleton instance
 *
 * @example
 * ```
 *  class WindowManager : public Singleton<WindowManager> {
 *  public:
 *    friend class Singleton<WindowManager>;
 *
 *  protected:
 *    WindowManager();
 *    ~WindowManager();
 *  }
 * ```
 *
 * @tparam TClass Derived class.
 */
template <class TClass> class Singleton {
public:
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

  /**
   * @brief Instance getter.
   * @return Pointer to instance of `TClass`.
   */
  static TClass &getInstance();

protected:
  Singleton() = default;
  ~Singleton() = default;

private:
  struct Deleter {
    void operator()(const TClass *ptr) const { delete ptr; }
  };

  using UniqueTClass = std::unique_ptr<TClass, Deleter>;

  static UniqueTClass &getStorage();
};

template <class TClass> TClass &Singleton<TClass>::getInstance() {
  UniqueTClass &storage = getStorage();
  if (!storage)
    storage.reset(new TClass());
  return *storage;
}

template <class TClass>
typename Singleton<TClass>::UniqueTClass &Singleton<TClass>::getStorage() {
  static UniqueTClass storage;
  return storage;
}

} // namespace Q