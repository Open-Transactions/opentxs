// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "ottest/Basic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
struct PaymentCodeFixture {
    static constexpr auto account_ = ot::Bip32Index{0};
    static constexpr auto index_ = ot::Bip32Index{0};

    auto blinding_key_public()
        -> const ot::crypto::asymmetric::key::EllipticCurve&;
    auto blinding_key_secret(
        const ot::api::session::Client& api,
        const ot::blockchain::Type chain,
        const ot::PasswordPrompt& reason)
        -> const ot::crypto::asymmetric::key::EllipticCurve&;
    auto blinding_key_secret(
        const ot::api::session::Client& api,
        const ot::UnallocatedCString& privateKey,
        const ot::PasswordPrompt& reason)
        -> const ot::crypto::asymmetric::key::EllipticCurve&;
    auto payment_code_public(
        const ot::api::Session& api,
        const ot::UnallocatedCString& base58) -> const ot::PaymentCode&;
    auto payment_code_secret(
        const ot::api::Session& api,
        const std::uint8_t version,
        const ot::PasswordPrompt& reason) -> const ot::PaymentCode&;
    auto seed(
        const ot::api::Session& api,
        const std::string_view wordList,
        const ot::PasswordPrompt& reason) -> const ot::UnallocatedCString&;

    auto cleanup() -> void;

    ~PaymentCodeFixture() { cleanup(); }

private:
    std::optional<ot::UnallocatedCString> seed_{};
    std::optional<ot::PaymentCode> pc_secret_{};
    std::optional<ot::PaymentCode> pc_public_{};
    std::optional<ot::crypto::asymmetric::key::EllipticCurve>
        blind_key_secret_{};
    std::optional<ot::crypto::asymmetric::key::EllipticCurve>
        blind_key_public_{};

    auto bip44_path(
        const ot::api::session::Client& api,
        const ot::blockchain::Type chain,
        ot::Writer&& destination) const -> bool;
};

class PC_Fixture_Base : virtual public ::testing::Test
{
public:
    static PaymentCodeFixture user_1_;
    static PaymentCodeFixture user_2_;

    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;
    const ot::UnallocatedCString& alice_seed_;
    const ot::UnallocatedCString& bob_seed_;
    const ot::PaymentCode& alice_pc_secret_;
    const ot::PaymentCode& alice_pc_public_;
    const ot::PaymentCode& bob_pc_secret_;
    const ot::PaymentCode& bob_pc_public_;

    virtual auto Shutdown() noexcept -> void;

    PC_Fixture_Base(
        const std::uint8_t aliceVersion,
        const std::uint8_t bobVersion,
        const ot::UnallocatedCString& aliceBip39,
        const ot::UnallocatedCString& bobBip39,
        const ot::UnallocatedCString& aliceExpectedPC,
        const ot::UnallocatedCString& bobExpectedPC) noexcept;
};
}  // namespace ottest
