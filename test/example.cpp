#include <gtest/gtest.h>


TEST(example, passing_test) {
    EXPECT_EQ(true, true);
}

TEST(example, failing_test) {
    EXPECT_EQ(true, false);
}
