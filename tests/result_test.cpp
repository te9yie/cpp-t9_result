#include <gtest/gtest.h>
#include <t9_result/prelude.h>

#include <cstddef>
#include <utility>

namespace {

using namespace t9_result;

class NoncopyableObject {
 private:
  NoncopyableObject(const NoncopyableObject&) = delete;
  NoncopyableObject& operator=(const NoncopyableObject&) = delete;

 private:
  int m_id = 0;

 public:
  explicit NoncopyableObject(int id) : m_id(id) {}

  ~NoncopyableObject() {}

  NoncopyableObject(NoncopyableObject&& other) {
    swap(other);
  }

  NoncopyableObject& operator=(NoncopyableObject&& other) {
    NoncopyableObject(std::move(other)).swap(*this);
    return *this;
  }

  int id() const {
    return m_id;
  }

  void swap(NoncopyableObject& other) {
    using std::swap;
    swap(m_id, other.m_id);
  }

  friend void swap(NoncopyableObject& lhs, NoncopyableObject& rhs) {
    lhs.swap(rhs);
  }
};

/**
 * @brief make_ok関数のテスト
 *
 * - 基本的な型（int）での成功値の生成と取得をテスト
 * - ムーブ専用型（NoncopyableObject）での成功値の生成と取得をテスト
 */
TEST(ResultTest, MakeOk) {
  {
    Result<int, int> result = make_ok(42);
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.unwrap(), 42);
    EXPECT_EQ(result.ref_ok(), 42);
  }
  {
    Result<NoncopyableObject, int> result = make_ok_with<NoncopyableObject>(42);
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.unwrap().id(), 42);
    EXPECT_EQ(result.ref_ok().id(), 0) << "NoncopyableObject should be moved";
  }
}

/**
 * @brief make_err関数のテスト
 *
 * - 基本的な型（int）での失敗値の生成と取得をテスト
 * - ムーブ専用型（NoncopyableObject）での失敗値の生成と取得をテスト
 */
TEST(ResultTest, MakeErr) {
  {
    Result<int, int> result = make_err(42);
    EXPECT_TRUE(result.is_err());
    EXPECT_EQ(result.unwrap_err(), 42);
    EXPECT_EQ(result.ref_err(), 42);
  }
  {
    Result<int, NoncopyableObject> result =
        make_err_with<NoncopyableObject>(42);
    EXPECT_TRUE(result.is_err());
    EXPECT_EQ(result.unwrap_err().id(), 42);
    EXPECT_EQ(result.ref_err().id(), 0) << "NoncopyableObject should be moved";
  }
}

/**
 * @brief unwrap_or関数のテスト
 *
 * - 成功値を保持している場合、その値が返されることをテスト
 * - 失敗値を保持している場合、デフォルト値が返されることをテスト
 * - ムーブ専用型での動作をテスト
 */
TEST(ResultTest, UnwrapOr) {
  {
    Result<int, int> result = make_ok(42);
    EXPECT_EQ(result.unwrap_or(43), 42);
  }
  {
    Result<int, int> result = make_err(42);
    EXPECT_EQ(result.unwrap_or(43), 43);
  }
  {
    Result<NoncopyableObject, int> result = make_ok_with<NoncopyableObject>(42);
    EXPECT_EQ(result.unwrap_or(NoncopyableObject(43)).id(), 42);
  }
  {
    Result<NoncopyableObject, int> result = make_err(42);
    EXPECT_EQ(result.unwrap_or(NoncopyableObject(43)).id(), 43);
  }
}

/**
 * @brief map関数のテスト
 *
 * - 成功値に対して変換関数が適用されることをテスト
 * - 失敗値に対して変換関数が適用されないことをテスト
 */
TEST(ResultTest, Map) {
  {
    Result<int, int> result = make_ok(42);
    auto mapped = result.map([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.is_ok());
    EXPECT_EQ(mapped.unwrap(), 84);
  }
  {
    Result<int, int> result = make_err(42);
    auto mapped = result.map([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.is_err());
    EXPECT_EQ(mapped.unwrap_err(), 42);
  }
}

/**
 * @brief map_err関数のテスト
 *
 * - 成功値に対して変換関数が適用されないことをテスト
 * - 失敗値に対して変換関数が適用されることをテスト
 */
TEST(ResultTest, MapErr) {
  {
    Result<int, int> result = make_ok(42);
    auto mapped = result.map_err([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.is_ok());
    EXPECT_EQ(mapped.unwrap(), 42);
  }
  {
    Result<int, int> result = make_err(42);
    auto mapped = result.map_err([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.is_err());
    EXPECT_EQ(mapped.unwrap_err(), 84);
  }
}

/**
 * @brief inspect_ok関数のテスト
 *
 * - 成功値に対して副作用関数が実行されることをテスト
 * - 失敗値に対して副作用関数が実行されないことをテスト
 */
TEST(ResultTest, InspectOk) {
  {
    int called = 0;
    Result<int, int> result = make_ok(42);
    result.inspect_ok([&called](int x) { called = x; });
    EXPECT_EQ(called, 42);
  }
  {
    int called = 0;
    Result<int, int> result = make_err(42);
    result.inspect_ok([&called](int x) { called = x; });
    EXPECT_EQ(called, 0);
  }
}

/**
 * @brief inspect_err関数のテスト
 *
 * - 成功値に対して副作用関数が実行されないことをテスト
 * - 失敗値に対して副作用関数が実行されることをテスト
 */
TEST(ResultTest, InspectErr) {
  {
    int called = 0;
    Result<int, int> result = make_ok(42);
    result.inspect_err([&called](int x) { called = x; });
    EXPECT_EQ(called, 0);
  }
  {
    int called = 0;
    Result<int, int> result = make_err(42);
    result.inspect_err([&called](int x) { called = x; });
    EXPECT_EQ(called, 42);
  }
}

/**
 * @brief and_then関数のテスト
 *
 * - 成功値に対して成功を返す関数を適用した場合の動作をテスト
 * - 成功値に対して失敗を返す関数を適用した場合の動作をテスト
 * - 失敗値に対して関数が適用されないことをテスト
 */
TEST(ResultTest, AndThen) {
  auto ok_fn = [](int x) -> Result<int, int> { return make_ok(x * 2); };
  auto err_fn = [](int x) -> Result<int, int> { return make_err(x * 2); };

  {
    Result<int, int> result = make_ok(42);
    auto chained = result.and_then(ok_fn);
    EXPECT_TRUE(chained.is_ok());
    EXPECT_EQ(chained.unwrap(), 84);
  }
  {
    Result<int, int> result = make_ok(42);
    auto chained = result.and_then(err_fn);
    EXPECT_TRUE(chained.is_err());
    EXPECT_EQ(chained.unwrap_err(), 84);
  }
  {
    Result<int, int> result = make_err(42);
    auto chained = result.and_then(ok_fn);
    EXPECT_TRUE(chained.is_err());
    EXPECT_EQ(chained.unwrap_err(), 42);
  }
}

}  // namespace
