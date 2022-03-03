// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

#include "Basic.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ottest
{
struct PaymentCodeFixture {
    static constexpr auto account_ = ot::Bip32Index{0};
    static constexpr auto index_ = ot::Bip32Index{0};

    auto blinding_key_public() -> const ot::crypto::key::EllipticCurve&;
    auto blinding_key_secret(
        const ot::api::session::Client& api,
        const ot::blockchain::Type chain,
        const ot::PasswordPrompt& reason)
        -> const ot::crypto::key::EllipticCurve&;
    auto blinding_key_secret(
        const ot::api::session::Client& api,
        const ot::UnallocatedCString& privateKey,
        const ot::PasswordPrompt& reason)
        -> const ot::crypto::key::EllipticCurve&;
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
    std::unique_ptr<const ot::crypto::key::EllipticCurve> blind_key_secret_{};
    std::unique_ptr<const ot::crypto::key::EllipticCurve> blind_key_public_{};

    auto bip44_path(
        const ot::api::session::Client& api,
        const ot::blockchain::Type chain,
        ot::AllocateOutput destination) const -> bool;
};

class PC_Fixture_Base : virtual public ::testing::Test
{
public:
    static PaymentCodeFixture user_1_;
    static PaymentCodeFixture user_2_;

    const ot::api::session::Client& api_;
    const ot::OTPasswordPrompt reason_;
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
