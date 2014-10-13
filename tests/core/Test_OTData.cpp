#include <gtest/gtest.h>
#include <opentxs/core/OTData.hpp>

using namespace opentxs;

namespace
{

struct Default_OTData : public ::testing::Test
{
    ot_data_t data_;
};

} // namespace

TEST_F(Default_OTData, default_accessors)
{
    ASSERT_TRUE(data_.data() == 0);
    ASSERT_TRUE(data_.size() == 0);
}

TEST(OTData, compare_equal_to_self)
{
    ot_data_t one = {'a', 'b', 'c', 'd'};
    ASSERT_TRUE(one == one);
}

TEST(OTData, compare_equal_to_other_same)
{
    ot_data_t one = {'a', 'b', 'c', 'd'};
    ot_data_t other = {'a', 'b', 'c', 'd'};
    ASSERT_TRUE(one == other);
}

TEST(OTData, compare_equal_to_other_different)
{
    ot_data_t one = {'a', 'b', 'c', 'd'};
    ot_data_t other = {'z', 'z', 'z', 'z'};
    ASSERT_FALSE(one == other);
}

TEST(OTData, compare_not_equal_to_self)
{
    ot_data_t one = {'a', 'b', 'c', 'd'};
    ASSERT_FALSE(one != one);
}

TEST(OTData, compare_not_equal_to_other_same)
{
    ot_data_t one = {'a', 'b', 'c', 'd'};
    ot_data_t other = {'a', 'b', 'c', 'd'};
    ASSERT_FALSE(one != other);
}

TEST(OTData, compare_not_equal_to_other_different)
{
    ot_data_t one = {'a', 'b', 'c', 'd'};
    ot_data_t other = {'z', 'z', 'z', 'z'};
    ASSERT_TRUE(one != other);
}
