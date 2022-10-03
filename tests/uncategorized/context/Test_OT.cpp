// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string_view>

#include "ottest/Basic.hpp"
#include "ottest/mocks/util/PasswordCallback.hpp"
#include "util/license/License.hpp"

namespace ottest
{
namespace ot_mocks = common::mocks::util;
using ::testing::StrictMock;
using namespace std::literals;

TEST(OT_suite, ShouldThrowAnExceptionDuringGettingUninitializeContext)
{
    const auto expected = "Context is not initialized"sv;
    auto error_message = ot::CString{};

    try {
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    const auto prefix = std::string_view{
        error_message.data(), std::min(expected.size(), error_message.size())};

    EXPECT_EQ(expected, prefix);

    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeAndGetValidContext)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

    try {
        opentxs::InitContext();
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeContextWithArgs)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

    try {
        opentxs::InitContext(ottest::Args(true));
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeContextWithInvalidPasswordCallback)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;
    try {
        opentxs::InitContext(nullptr);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }
    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeContextWithValidPasswordCaller)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

    try {
        opentxs::InitContext(&caller);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldInitializeContextWithArgsAndInvalidPasswordCallerWithoutThrowingAnException)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

    try {
        opentxs::InitContext(ottest::Args(true), nullptr);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldInitializeContextWithArgsAndValidPasswordCallerWithoutThrowingAnException)
{
    const ot::UnallocatedCString expected = "";
    ot::UnallocatedCString error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

    try {
        opentxs::InitContext(ottest::Args(true), &caller);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldDoubleDefaultInitializeContextAndThrowAnException)
{
    const ot::UnallocatedCString expected = "Context is already initialized";
    ot::UnallocatedCString error_message;
    try {
        opentxs::InitContext();
        opentxs::InitContext();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldDoubleInitializeContextWithArgsAndThrowAnException)
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
    ShouldDoubleInitializeContextWithValidPasswordCallerAndThrowAnException)
{
    const ot::UnallocatedCString expected = "Context is already initialized";
    ot::UnallocatedCString error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

    try {
        opentxs::InitContext(&caller);
        opentxs::InitContext(&caller);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldDoubleInitializeContextWithArgsAndValidPasswordCallerAndThrowAnException)
{
    const ot::UnallocatedCString expected = "Context is already initialized";
    ot::UnallocatedCString error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

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
    ShouldDoubleInitializeContextWithArgsAndNotValidPasswordCallerAndThrowAnException)
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

TEST(OT_suite, ShouldReturnValidLicenseMap)
{
    const auto expected_license_map = std::invoke([]() {
        auto out = opentxs::LicenseMap{};
        opentxs::license_argon(out);
        opentxs::license_base58(out);
        opentxs::license_base64(out);
        opentxs::license_bech32(out);
        opentxs::license_chaiscript(out);
        opentxs::license_irrxml(out);
        opentxs::license_libguarded(out);
        opentxs::license_lucre(out);
        opentxs::license_opentxs(out);
        opentxs::license_packetcrypt(out);
        opentxs::license_protobuf(out);
        opentxs::license_secp256k1(out);
        opentxs::license_simpleini(out);
        return out;
    });

    EXPECT_EQ(expected_license_map, opentxs::LicenseData());
}

}  // namespace ottest
