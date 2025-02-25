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

}  // namespace
