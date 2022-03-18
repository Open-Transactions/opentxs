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
TEST(NaggyMock, constructor) { ::ottest::mock::Interface naggyMock; }

TEST(NaggyMock, naggyMockOnFunction)
{
    ::ottest::mock::Interface naggyMock;
    ON_CALL(naggyMock, someFunction(0)).WillByDefault(::testing::Return(false));
    ON_CALL(naggyMock, someFunction(1)).WillByDefault(::testing::Return(true));

    EXPECT_CALL(naggyMock, someFunction(0)).Times(1);
    EXPECT_CALL(naggyMock, someFunction(1)).Times(1);

    EXPECT_TRUE(naggyMock.someFunction(1));
    EXPECT_FALSE(naggyMock.someFunction(0));

    // uninteresting call
    ON_CALL(naggyMock, someConstFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(naggyMock.someConstFunction(2));
}

TEST(NaggyMock, naggyMockOnConstFunction)
{
    ::ottest::mock::Interface naggyMock;
    ON_CALL(naggyMock, someConstFunction(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(naggyMock, someConstFunction(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(naggyMock, someConstFunction(0)).Times(1);
    EXPECT_CALL(naggyMock, someConstFunction(1)).Times(1);

    EXPECT_TRUE(naggyMock.someConstFunction(1));
    EXPECT_FALSE(naggyMock.someConstFunction(0));

    // uninteresting call
    ON_CALL(naggyMock, someFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(naggyMock.someFunction(2));
}

TEST(NaggyMock, naggyMockOnNoExceptFunction)
{
    ::ottest::mock::Interface naggyMock;
    ON_CALL(naggyMock, someFunctionWithNoExcept(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(naggyMock, someFunctionWithNoExcept(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(naggyMock, someFunctionWithNoExcept(0)).Times(1);
    EXPECT_CALL(naggyMock, someFunctionWithNoExcept(1)).Times(1);

    EXPECT_TRUE(naggyMock.someFunctionWithNoExcept(1));
    EXPECT_FALSE(naggyMock.someFunctionWithNoExcept(0));

    // uninteresting call
    ON_CALL(naggyMock, someFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(naggyMock.someFunction(2));
}

TEST(NaggyMock, naggyMockOnNoExceptConstFunction)
{
    ::ottest::mock::Interface naggyMock;
    ON_CALL(naggyMock, someConstFunctionWithNoExcept(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(naggyMock, someConstFunctionWithNoExcept(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(naggyMock, someConstFunctionWithNoExcept(0)).Times(1);
    EXPECT_CALL(naggyMock, someConstFunctionWithNoExcept(1)).Times(1);

    EXPECT_TRUE(naggyMock.someConstFunctionWithNoExcept(1));
    EXPECT_FALSE(naggyMock.someConstFunctionWithNoExcept(0));

    // uninteresting call
    ON_CALL(naggyMock, someFunction(::testing::_))
        .WillByDefault(::testing::Return(true));
    EXPECT_TRUE(naggyMock.someFunction(2));
}
}  // namespace ottest
