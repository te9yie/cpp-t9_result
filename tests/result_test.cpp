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

// 基本的な型とムーブ専用型での成功値の生成と取得をテスト
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

// 基本的な型とムーブ専用型での失敗値の生成と取得をテスト
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

// 成功値と失敗値の場合のunwrap_orの動作をテスト
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

// 成功値と失敗値に対するmap関数の動作をテスト
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

// 成功値と失敗値に対するmap_err関数の動作をテスト
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

// 成功値と失敗値に対するinspect_ok関数の動作をテスト
TEST(ResultTest, InspectOk) {
  {
    int called = 0;
    Result<int, int> result = make_ok(42);
    auto& ref = result.inspect_ok([&called](int x) { called = x; });
    EXPECT_EQ(called, 42);
    EXPECT_EQ(&ref, &result) << "inspect_ok should return reference to self";

    // メソッドチェーンのテスト
    called = 0;
    result.inspect_ok([&called](int x) { called = x; })
        .inspect_ok([&called](int x) { called += x; });
    EXPECT_EQ(called, 84) << "Method chaining should work";
  }
  {
    int called = 0;
    Result<int, int> result = make_err(42);
    auto& ref = result.inspect_ok([&called](int x) { called = x; });
    EXPECT_EQ(called, 0);
    EXPECT_EQ(&ref, &result) << "inspect_ok should return reference to self";
  }
}

// 成功値と失敗値に対するinspect_err関数の動作をテスト
TEST(ResultTest, InspectErr) {
  {
    int called = 0;
    Result<int, int> result = make_ok(42);
    auto& ref = result.inspect_err([&called](int x) { called = x; });
    EXPECT_EQ(called, 0);
    EXPECT_EQ(&ref, &result) << "inspect_err should return reference to self";
  }
  {
    int called = 0;
    Result<int, int> result = make_err(42);
    auto& ref = result.inspect_err([&called](int x) { called = x; });
    EXPECT_EQ(called, 42);
    EXPECT_EQ(&ref, &result) << "inspect_err should return reference to self";

    // メソッドチェーンのテスト
    called = 0;
    result.inspect_err([&called](int x) { called = x; })
        .inspect_err([&called](int x) { called += x; });
    EXPECT_EQ(called, 84) << "Method chaining should work";
  }
}

// 成功値と失敗値に対するand_then関数の動作をテスト
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

// void型のResultの基本的な動作をテスト
TEST(ResultTest, VoidResult) {
  {
    Result<void, int> result = make_ok();
    EXPECT_TRUE(result.is_ok());
    result.unwrap();  // アサーションが発生しないことを確認
  }
  {
    Result<void, int> result = make_err(42);
    EXPECT_TRUE(result.is_err());
    EXPECT_EQ(result.unwrap_err(), 42);
    EXPECT_EQ(result.ref_err(), 42);
  }
}

// void型のResultに対するmap関数の動作をテスト
TEST(ResultTest, VoidResultMap) {
  {
    Result<void, int> result = make_ok();
    int called = 0;
    auto mapped = result.map([&called]() {
      called = 42;
      return called;
    });
    EXPECT_TRUE(mapped.is_ok());
    EXPECT_EQ(mapped.unwrap(), 42);
    EXPECT_EQ(called, 42);
  }
  {
    Result<void, int> result = make_err(42);
    int called = 0;
    auto mapped = result.map([&called]() {
      called = 43;
      return called;
    });
    EXPECT_TRUE(mapped.is_err());
    EXPECT_EQ(mapped.unwrap_err(), 42);
    EXPECT_EQ(called, 0) << "Map function should not be called on error";
  }
}

// void型のResultに対するmap_err関数の動作をテスト
TEST(ResultTest, VoidResultMapErr) {
  {
    Result<void, int> result = make_ok();
    auto mapped = result.map_err([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.is_ok());
  }
  {
    Result<void, int> result = make_err(42);
    auto mapped = result.map_err([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.is_err());
    EXPECT_EQ(mapped.unwrap_err(), 84);
  }
}

// void型のResultに対するinspect_ok関数の動作をテスト
TEST(ResultTest, VoidResultInspectOk) {
  {
    int called = 0;
    Result<void, int> result = make_ok();
    auto& ref = result.inspect_ok([&called]() { called = 42; });
    EXPECT_EQ(called, 42);
    EXPECT_EQ(&ref, &result) << "inspect_ok should return reference to self";

    // メソッドチェーンのテスト
    called = 0;
    result.inspect_ok([&called]() { called = 42; }).inspect_ok([&called]() {
      called += 42;
    });
    EXPECT_EQ(called, 84) << "Method chaining should work";
  }
  {
    int called = 0;
    Result<void, int> result = make_err(42);
    auto& ref = result.inspect_ok([&called]() { called = 42; });
    EXPECT_EQ(called, 0);
    EXPECT_EQ(&ref, &result) << "inspect_ok should return reference to self";
  }
}

// void型のResultに対するinspect_err関数の動作をテスト
TEST(ResultTest, VoidResultInspectErr) {
  {
    int called = 0;
    Result<void, int> result = make_ok();
    auto& ref = result.inspect_err([&called](int x) { called = x; });
    EXPECT_EQ(called, 0);
    EXPECT_EQ(&ref, &result) << "inspect_err should return reference to self";
  }
  {
    int called = 0;
    Result<void, int> result = make_err(42);
    auto& ref = result.inspect_err([&called](int x) { called = x; });
    EXPECT_EQ(called, 42);
    EXPECT_EQ(&ref, &result) << "inspect_err should return reference to self";

    // メソッドチェーンのテスト
    called = 0;
    result.inspect_err([&called](int x) { called = x; })
        .inspect_err([&called](int x) { called += x; });
    EXPECT_EQ(called, 84) << "Method chaining should work";
  }
}

// void型のResultに対するand_then関数の動作をテスト
TEST(ResultTest, VoidResultAndThen) {
  auto ok_fn = []() -> Result<int, int> { return make_ok(42); };
  auto err_fn = []() -> Result<int, int> { return make_err(42); };
  auto void_ok_fn = []() -> Result<void, int> { return make_ok(); };
  auto void_err_fn = []() -> Result<void, int> { return make_err(42); };

  {
    Result<void, int> result = make_ok();
    auto chained = result.and_then(ok_fn);
    EXPECT_TRUE(chained.is_ok());
    EXPECT_EQ(chained.unwrap(), 42);
  }
  {
    Result<void, int> result = make_ok();
    auto chained = result.and_then(err_fn);
    EXPECT_TRUE(chained.is_err());
    EXPECT_EQ(chained.unwrap_err(), 42);
  }
  {
    Result<void, int> result = make_err(42);
    auto chained = result.and_then(ok_fn);
    EXPECT_TRUE(chained.is_err());
    EXPECT_EQ(chained.unwrap_err(), 42);
  }
  {
    Result<void, int> result = make_ok();
    auto chained = result.and_then(void_ok_fn);
    EXPECT_TRUE(chained.is_ok());
  }
  {
    Result<void, int> result = make_ok();
    auto chained = result.and_then(void_err_fn);
    EXPECT_TRUE(chained.is_err());
    EXPECT_EQ(chained.unwrap_err(), 42);
  }
}

}  // namespace
