#pragma once

#include <cassert>
#include <variant>

namespace t9_result {

/**
 * @brief 成功値を表す型
 * @tparam T 成功値の型
 */
template <typename T>
struct Ok {
  T m_value;

  Ok(const T& value) : m_value(value) {}
  Ok(T&& value) : m_value(std::move(value)) {}
};

/**
 * @brief 成功値からOk型を生成するヘルパー関数
 * @tparam T 成功値の型
 * @param value 成功値
 * @return Ok<T> 成功値をラップしたOk型
 */
template <typename T>
inline Ok<T> make_ok(T value) {
  return value;
}

/**
 * @brief 引数から直接Ok型のオブジェクトを構築するヘルパー関数
 * @tparam T 構築する型
 * @tparam Args コンストラクタ引数の型
 * @param args コンストラクタに渡す引数
 * @return Ok<T> 構築されたオブジェクトをラップしたOk型
 */
template <typename T, typename... Args>
inline Ok<T> make_ok_with(Args&&... args) {
  return T{std::forward<Args>(args)...};
}

/**
 * @brief 失敗値を表す型
 * @tparam T 失敗値の型
 */
template <typename T>
struct Err {
  T m_value;

  Err(const T& value) : m_value(value) {}
  Err(T&& value) : m_value(std::move(value)) {}
};

/**
 * @brief 失敗値からErr型を生成するヘルパー関数
 * @tparam T 失敗値の型
 * @param value 失敗値
 * @return Err<T> 失敗値をラップしたErr型
 */
template <typename T>
inline Err<T> make_err(T value) {
  return value;
}

/**
 * @brief 引数から直接Err型のオブジェクトを構築するヘルパー関数
 * @tparam E 構築する型
 * @tparam Args コンストラクタ引数の型
 * @param args コンストラクタに渡す引数
 * @return Err<E> 構築されたオブジェクトをラップしたErr型
 */
template <typename E, typename... Args>
inline Err<E> make_err_with(Args&&... args) {
  return E{std::forward<Args>(args)...};
}

/**
 * @brief Result型
 * @tparam T 成功値の型
 * @tparam E 失敗値の型
 *
 * 成功値(T)もしくは失敗値(E)を持つ型です。
 */
template <typename T, typename E>
class Result final {
 private:
  std::variant<std::monostate, Ok<T>, Err<E>> m_value;

 public:
  /**
   * @brief 成功値からResultを生成するコンストラクタ
   * @param ok 成功値をラップしたOk型
   */
  Result(Ok<T> ok) : m_value(std::move(ok)) {}

  /**
   * @brief 失敗値からResultを生成するコンストラクタ
   * @param err 失敗値をラップしたErr型
   */
  Result(Err<E> err) : m_value(std::move(err)) {}

  /**
   * @brief 成功値を保持しているか確認
   * @return bool 成功値を保持している場合true
   */
  bool is_ok() const {
    return std::holds_alternative<Ok<T>>(m_value);
  }

  /**
   * @brief 失敗値を保持しているか確認
   * @return bool 失敗値を保持している場合true
   */
  bool is_err() const {
    return std::holds_alternative<Err<E>>(m_value);
  }

  /**
   * @brief 成功値を取得（所有権を移動）
   * @return T 成功値
   * @note 失敗値を保持している場合はアサーション違反
   */
  T unwrap() {
    assert(is_ok());
    return std::move(std::get<Ok<T>>(m_value).m_value);
  }

  /**
   * @brief 成功値を取得、失敗時はデフォルト値を返す
   * @param default_value 失敗時に返すデフォルト値
   * @return T 成功値もしくはデフォルト値
   */
  T unwrap_or(T&& default_value) {
    if (is_ok()) {
      return unwrap();
    }
    return std::forward<T>(default_value);
  }

  /**
   * @brief 失敗値を取得（所有権を移動）
   * @return E 失敗値
   * @note 成功値を保持している場合はアサーション違反
   */
  E unwrap_err() {
    assert(is_err());
    return std::move(std::get<Err<E>>(m_value).m_value);
  }

  /**
   * @brief 成功値への参照を取得
   * @return T& 成功値への参照
   * @note 失敗値を保持している場合はアサーション違反
   */
  T& ref_ok() {
    assert(is_ok());
    return std::get<Ok<T>>(m_value).m_value;
  }

  /**
   * @brief 成功値への const 参照を取得
   * @return const T& 成功値への const 参照
   * @note 失敗値を保持している場合はアサーション違反
   */
  const T& ref_ok() const {
    assert(is_ok());
    return std::get<Ok<T>>(m_value).m_value;
  }

  /**
   * @brief 失敗値への参照を取得
   * @return E& 失敗値への参照
   * @note 成功値を保持している場合はアサーション違反
   */
  E& ref_err() {
    assert(is_err());
    return std::get<Err<E>>(m_value).m_value;
  }

  /**
   * @brief 失敗値への const 参照を取得
   * @return const E& 失敗値への const 参照
   * @note 成功値を保持している場合はアサーション違反
   */
  const E& ref_err() const {
    assert(is_err());
    return std::get<Err<E>>(m_value).m_value;
  }

  /**
   * @brief 成功値に関数を適用して新しいResult型を生成
   * @tparam F 適用する関数の型
   * @param f 適用する関数
   * @return Result<decltype(f(T)), E> 関数適用後の新しいResult型
   *
   * 成功値を保持している場合は関数fを適用し、その結果を新しいResultとして返します。
   * 失敗値を保持している場合は、失敗値をそのまま保持した新しいResultを返します。
   */
  template <typename F>
  auto map(F&& f) -> Result<decltype(f(std::declval<T>())), E> {
    Result self = std::move(*this);
    if (self.is_ok()) {
      return make_ok(f(std::get<Ok<T>>(self.m_value).m_value));
    }
    return Err<E>(std::get<Err<E>>(self.m_value).m_value);
  }

  /**
   * @brief 失敗値に関数を適用して新しいResult型を生成
   * @tparam F 適用する関数の型
   * @param f 適用する関数
   * @return Result<T, decltype(f(E))> 関数適用後の新しいResult型
   *
   * 失敗値を保持している場合は関数fを適用し、その結果を新しいResultとして返します。
   * 成功値を保持している場合は、成功値をそのまま保持した新しいResultを返します。
   */
  template <typename F>
  auto map_err(F&& f) -> Result<T, decltype(f(std::declval<E>()))> {
    Result self = std::move(*this);
    if (self.is_err()) {
      return make_err(f(std::get<Err<E>>(self.m_value).m_value));
    }
    return Ok<T>(std::get<Ok<T>>(self.m_value).m_value);
  }

  /**
   * @brief 成功値に関数を適用し、元のResultを返す
   * @tparam F 適用する関数の型
   * @param f 適用する関数
   * @return Result 元のResult
   *
   * 成功値を保持している場合のみ関数fを適用します。
   */
  template <typename F>
  Result& inspect_ok(F&& f) {
    if (is_ok()) {
      f(std::get<Ok<T>>(m_value).m_value);
    }
    return *this;
  }

  /**
   * @brief 失敗値に関数を適用し、元のResultを返す
   * @tparam F 適用する関数の型
   * @param f 適用する関数
   * @return Result 元のResult
   *
   * 失敗値を保持している場合のみ関数fを適用します。
   */
  template <typename F>
  Result& inspect_err(F&& f) {
    if (is_err()) {
      f(std::get<Err<E>>(m_value).m_value);
    }
    return *this;
  }

  /**
   * @brief 成功値に関数を適用してチェーン処理を行う
   * @tparam F 適用する関数の型
   * @param f Result型を返す関数
   * @return decltype(f(T)) 関数fの戻り値型
   *
   * 成功値を保持している場合は関数fを適用し、その結果のResultを返します。
   * 失敗値を保持している場合は、失敗値をそのまま保持した新しいResultを返します。
   */
  template <typename F>
  auto and_then(F&& f) -> decltype(f(std::declval<T>())) {
    Result self = std::move(*this);
    if (self.is_ok()) {
      return f(std::get<Ok<T>>(self.m_value).m_value);
    }
    return Err<E>(std::get<Err<E>>(self.m_value).m_value);
  }
};

}  // namespace t9_result
