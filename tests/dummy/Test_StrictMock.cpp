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
TEST(StrictMock, constructor)
{
    ::testing::StrictMock<::ottest::mock::Interface> strictMock;
}

TEST(StrictMock, strictMockOnFunction)
{
    ::testing::StrictMock<::ottest::mock::Interface> strictMock;
    ON_CALL(strictMock, someFunction(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(strictMock, someFunction(1)).WillByDefault(::testing::Return(true));

    EXPECT_CALL(strictMock, someFunction(0)).Times(1);
    EXPECT_CALL(strictMock, someFunction(1)).Times(1);

    EXPECT_TRUE(strictMock.someFunction(1));
    EXPECT_FALSE(strictMock.someFunction(0));
}

TEST(StrictMock, strictMockOnConstFunction)
{
    ::testing::StrictMock<::ottest::mock::Interface> strictMock;
    ON_CALL(strictMock, someConstFunction(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(strictMock, someConstFunction(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(strictMock, someConstFunction(0)).Times(1);
    EXPECT_CALL(strictMock, someConstFunction(1)).Times(1);

    EXPECT_TRUE(strictMock.someConstFunction(1));
    EXPECT_FALSE(strictMock.someConstFunction(0));
}

TEST(StrictMock, strictMockOnNoExceptFunction)
{
    ::testing::StrictMock<::ottest::mock::Interface> strictMock;
    ON_CALL(strictMock, someFunctionWithNoExcept(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(strictMock, someFunctionWithNoExcept(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(strictMock, someFunctionWithNoExcept(0)).Times(1);
    EXPECT_CALL(strictMock, someFunctionWithNoExcept(1)).Times(1);

    EXPECT_TRUE(strictMock.someFunctionWithNoExcept(1));
    EXPECT_FALSE(strictMock.someFunctionWithNoExcept(0));
}

TEST(StrictMock, strictMockOnNoExceptConstFunction)
{
    ::testing::StrictMock<::ottest::mock::Interface> strictMock;
    ON_CALL(strictMock, someConstFunctionWithNoExcept(0))
        .WillByDefault(::testing::Return(false));
    ON_CALL(strictMock, someConstFunctionWithNoExcept(1))
        .WillByDefault(::testing::Return(true));

    EXPECT_CALL(strictMock, someConstFunctionWithNoExcept(0)).Times(1);
    EXPECT_CALL(strictMock, someConstFunctionWithNoExcept(1)).Times(1);

    EXPECT_TRUE(strictMock.someConstFunctionWithNoExcept(1));
    EXPECT_FALSE(strictMock.someConstFunctionWithNoExcept(0));
}
}  // namespace ottest
