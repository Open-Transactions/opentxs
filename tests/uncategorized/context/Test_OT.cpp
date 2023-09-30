// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>
#include <stdexcept>
#include <string_view>

#include "ottest/Basic.hpp"
#include "ottest/mocks/util/PasswordCallback.hpp"

namespace ottest
{
namespace ot_mocks = common::mocks::util;
using ::testing::StrictMock;
using namespace std::literals;

// NOTE calling opentxs::InitContext without passing an opentxs::Options
// argument in a unit test is not allowed.

TEST(OT_suite, ShouldSuccessfullyInitializeContext)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

    try {
        opentxs::InitContext(ottest::Args(true));
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldInitializeContextAndInvalidPasswordCallerWithoutThrowingAnException)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

    try {
        opentxs::InitContext(ottest::Args(true), nullptr);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldInitializeContextAndValidPasswordCallerWithoutThrowingAnException)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"  // NOLINT
    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);
#pragma GCC diagnostic pop

    try {
        opentxs::InitContext(ottest::Args(true), &caller);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldDoubleInitializeContextAndThrowAnException)
{
    const ot::UnallocatedCString expected = "Context is already initialized";
    ot::UnallocatedCString error_message;
    try {
        opentxs::InitContext(ottest::Args(true));
        opentxs::InitContext(ottest::Args(true));
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldDoubleInitializeContextAndValidPasswordCallerAndThrowAnException)
{
    const ot::UnallocatedCString expected = "Context is already initialized";
    ot::UnallocatedCString error_message;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"  // NOLINT
    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);
#pragma GCC diagnostic pop

    try {
        opentxs::InitContext(ottest::Args(true), &caller);
        opentxs::InitContext(ottest::Args(true), &caller);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldDoubleInitializeContextAndNotValidPasswordCallerAndThrowAnException)
{
    const ot::UnallocatedCString expected = "Context is already initialized";
    ot::UnallocatedCString error_message;

    try {
        opentxs::InitContext(ottest::Args(true), nullptr);
        opentxs::InitContext(ottest::Args(true), nullptr);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}
}  // namespace ottest
