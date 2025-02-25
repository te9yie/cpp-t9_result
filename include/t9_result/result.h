#pragma once

#include <cassert>
#include <variant>

namespace t9_result {

/**
 * @brief 成功値を表す型
 */
template <typename T>
struct Ok {
  T m_value;

  Ok(const T& value) : m_value(value) {}
  Ok(T&& value) : m_value(std::move(value)) {}
};

template <typename T>
inline Ok<T> make_ok(T value) {
  return value;
}

template <typename T, typename... Args>
inline Ok<T> make_ok_with(Args&&... args) {
  return T{std::forward<Args>(args)...};
}

/**
 * @brief 失敗値を表す型
 */
template <typename T>
struct Err {
  T m_value;

  Err(const T& value) : m_value(value) {}
  Err(T&& value) : m_value(std::move(value)) {}
};

template <typename T>
inline Err<T> make_err(T value) {
  return value;
}

template <typename E, typename... Args>
inline Err<E> make_err_with(Args&&... args) {
  return E{std::forward<Args>(args)...};
}

/**
 * @brief Result型
 *
 * 成功値(T)もしくは失敗値(E)を持つ型
 */
template <typename T, typename E>
class Result final {
 private:
  std::variant<std::monostate, Ok<T>, Err<E>> m_value;

 public:
  /// @brief 成功値からResultを生成する
  Result(Ok<T> ok) : m_value(std::move(ok)) {}

  /// @brief 失敗値からResultを生成する
  Result(Err<E> err) : m_value(std::move(err)) {}

  /// @brief 成功値を保持しているか
  bool is_ok() const {
    return std::holds_alternative<Ok<T>>(m_value);
  }

  /// @brief 失敗値を保持しているか
  bool is_err() const {
    return std::holds_alternative<Err<E>>(m_value);
  }

  /// @brief 成功値を取得する（所有権を移動）
  T unwrap() {
    assert(is_ok());
    return std::move(std::get<Ok<T>>(m_value).m_value);
  }

  /// @brief 失敗値を取得する（所有権を移動）
  E unwrap_err() {
    assert(is_err());
    return std::move(std::get<Err<E>>(m_value).m_value);
  }

  /// @brief 成功値を参照する
  T& ref_ok() {
    assert(is_ok());
    return std::get<Ok<T>>(m_value).m_value;
  }

  /// @brief 成功値を参照する
  const T& ref_ok() const {
    assert(is_ok());
    return std::get<Ok<T>>(m_value).m_value;
  }

  /// @brief 失敗値を参照する
  E& ref_err() {
    assert(is_err());
    return std::get<Err<E>>(m_value).m_value;
  }

  /// @brief 失敗値を参照する
  const E& ref_err() const {
    assert(is_err());
    return std::get<Err<E>>(m_value).m_value;
  }
};

}  // namespace t9_result
