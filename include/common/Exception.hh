#pragma once

#include <concepts>
#include <exception>
#include <string>

namespace Q {

class Exception : public std::exception {
public:
  /**
   * @brief Construct an exception with user-provided message.
   * @tparam TMessage `std::convertible_to` `std::string` type of the `message`
   * param.
   * @param message Exception message.
   */
  template <typename TMessage>
    requires std::convertible_to<TMessage, std::string>
  explicit Exception(TMessage &&message);

  ~Exception() noexcept override = default;

  [[nodiscard]] const char *what() const noexcept override;

protected:
  const std::string message;
};

template <typename TMessage>
  requires std::convertible_to<TMessage, std::string>
Exception::Exception(TMessage &&message)
    : message{std::forward<TMessage>(message)} {}

} // namespace Q
