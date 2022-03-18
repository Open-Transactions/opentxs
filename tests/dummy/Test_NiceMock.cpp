// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "dummy/Mocks/DummyInterfaceMock.hpp"

namespace ottest
{
TEST(NiceMock, constructor)
{
    ::testing::NiceMock<::ottest::mock::Interface> mockInstance;
}

TEST(NiceMock, niceMockOnFunction)
{
    ::testing::NiceMock<::ottest::mock::Interface> niceMock;
    ON_CALL(niceMock, someFunction(0)).WillByDefault(::testing::Return(false));
    ON_CALL(niceMock, someFunction(1)).WillByDefault(::testing::Return(true));

    EXPECT_CALL(niceMock, someFunction(0)).Times(1);
    EXPECT_CALL(niceMock, someFunction(1)).Times(1);

    EXPECT_TRUE(niceMock.someFunction(1));
    EXPECT_FALSE(niceMock.someFunction(0));

    // uninteresting call
    ON_CALL(niceMock, someConstFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(niceMock.someConstFunction(2));
}

TEST(NiceMock, niceMockOnConstFunction)
{
    ::testing::NiceMock<::ottest::mock::Interface> niceMock;
    ON_CALL(niceMock, someConstFunction(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(niceMock, someConstFunction(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(niceMock, someConstFunction(0)).Times(1);
    EXPECT_CALL(niceMock, someConstFunction(1)).Times(1);

    EXPECT_TRUE(niceMock.someConstFunction(1));
    EXPECT_FALSE(niceMock.someConstFunction(0));

    // uninteresting call
    ON_CALL(niceMock, someFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(niceMock.someFunction(2));
}

TEST(NiceMock, niceMockOnNoExceptFunction)
{
    ::testing::NiceMock<::ottest::mock::Interface> niceMock;
    ON_CALL(niceMock, someFunctionWithNoExcept(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(niceMock, someFunctionWithNoExcept(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(niceMock, someFunctionWithNoExcept(0)).Times(1);
    EXPECT_CALL(niceMock, someFunctionWithNoExcept(1)).Times(1);

    EXPECT_TRUE(niceMock.someFunctionWithNoExcept(1));
    EXPECT_FALSE(niceMock.someFunctionWithNoExcept(0));

    // uninteresting call
    ON_CALL(niceMock, someFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(niceMock.someFunction(2));
}

TEST(NiceMock, niceMockOnNoExceptConstFunction)
{
    ::testing::NiceMock<::ottest::mock::Interface> niceMock;
    ON_CALL(niceMock, someConstFunctionWithNoExcept(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(niceMock, someConstFunctionWithNoExcept(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(niceMock, someConstFunctionWithNoExcept(0)).Times(1);
    EXPECT_CALL(niceMock, someConstFunctionWithNoExcept(1)).Times(1);

    EXPECT_TRUE(niceMock.someConstFunctionWithNoExcept(1));
    EXPECT_FALSE(niceMock.someConstFunctionWithNoExcept(0));

    // uninteresting call
    ON_CALL(niceMock, someFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(niceMock.someFunction(2));
}
}  // namespace ottest
