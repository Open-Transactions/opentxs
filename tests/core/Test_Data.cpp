#include <gtest/gtest.h>
#include <string>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"
#include "opentxs/core/Data.hpp"

using namespace opentxs;

namespace
{

struct Default_Data : public ::testing::Test
{
  Data data_;
};

} // namespace

TEST_F(Default_Data, default_accessors)
{
  ASSERT_TRUE(data_.GetPointer() == 0);
  ASSERT_TRUE(data_.GetSize() == 0);
}

TEST(Data, compare_equal_to_self)
{
    Data one("abcd", 4);
    ASSERT_TRUE(one == one);
}

TEST(Data, compare_equal_to_other_same)
{
    Data one("abcd", 4);
    Data other("abcd", 4);
    ASSERT_TRUE(one == other);
}

TEST(Data, compare_equal_to_other_different)
{
    Data one("abcd", 4);
    Data other("zzzz", 4);
    ASSERT_FALSE(one == other);
}

TEST(Data, compare_not_equal_to_self)
{
    Data one("aaaa", 4);
    ASSERT_FALSE(one != one);
}

TEST(Data, compare_not_equal_to_other_same)
{
    Data one("abcd", 4);
    Data other("abcd", 4);
    ASSERT_FALSE(one != other);
}

TEST(Data, compare_not_equal_to_other_different)
{
    Data one("abcd", 4);
    Data other("zzzz", 4);
    ASSERT_TRUE(one != other);
}
