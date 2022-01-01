// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/key/HD.hpp"             // IWYU pragma: keep
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "paymentcode/VectorsV3.hpp"

namespace ottest
{
const ot::Nym_p nym_{};
const std::string seed_id_{};
using Pubkey = ot::Space;
using ExpectedKeys = std::pmr::vector<Pubkey>;
const ExpectedKeys external_{};
const ExpectedKeys internal_{};

class Test_BIP44 : public ::testing::Test
{
protected:
    static constexpr auto count_{1000u};
    static constexpr auto account_id_{
        "ot2xkvDhg67hCe4xvgz3KHj89JhEJp3UbLxFy2CunoZoKGCxhPYVkR9"};

    const ot::api::session::Client& api_;
    const ot::OTPasswordPrompt reason_;
    const ot::identifier::Nym& nym_id_;
    const ot::blockchain::crypto::HD& account_;

    Test_BIP44()
        : api_(ot::Context().StartClientSession(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
        , nym_id_([&]() -> const ot::identifier::Nym& {
            if (seed_id_.empty()) {
                const auto words =
                    api_.Factory().SecretFromText(GetVectors3().alice_.words_);
                const auto phrase = api_.Factory().Secret(0);
                const_cast<std::string&>(seed_id_) =
                    api_.Crypto().Seed().ImportSeed(
                        words,
                        phrase,
                        ot::crypto::SeedStyle::BIP39,
                        ot::crypto::Language::en,
                        reason_);
            }

            OT_ASSERT(0 < seed_id_.size());

            if (!nym_) {
                const_cast<ot::Nym_p&>(nym_) =
                    api_.Wallet().Nym({seed_id_, 0}, reason_, "Alice");

                OT_ASSERT(nym_);

                api_.Crypto().Blockchain().NewHDSubaccount(
                    nym_->ID(),
                    ot::blockchain::crypto::HDProtocol::BIP_44,
                    ot::blockchain::Type::UnitTest,
                    reason_);
            }

            OT_ASSERT(nym_);

            return nym_->ID();
        }())
        , account_(api_.Crypto()
                       .Blockchain()
                       .Account(nym_id_, ot::blockchain::Type::UnitTest)
                       .GetHD()
                       .at(0))
    {
    }
};

TEST_F(Test_BIP44, init)
{
    EXPECT_FALSE(seed_id_.empty());
    EXPECT_FALSE(nym_id_.empty());
    EXPECT_EQ(account_.ID().str(), account_id_);
    EXPECT_EQ(account_.Standard(), ot::blockchain::crypto::HDProtocol::BIP_44);
}

TEST_F(Test_BIP44, generate_expected_keys)
{
    auto& internal = const_cast<ExpectedKeys&>(internal_);
    auto& external = const_cast<ExpectedKeys&>(external_);
    internal.reserve(count_);
    external.reserve(count_);
    using EcdsaCurve = ot::EcdsaCurve;
    using Path = std::pmr::vector<ot::Bip32Index>;
    const auto MakePath = [&](auto change, auto index) -> Path {
        constexpr auto hard =
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED);
        constexpr auto purpose =
            static_cast<ot::Bip32Index>(ot::Bip43Purpose::HDWALLET) | hard;
        constexpr auto coin_type =
            static_cast<ot::Bip32Index>(ot::Bip44Type::TESTNET) | hard;
        constexpr auto account = ot::Bip32Index{0} | hard;

        return {purpose, coin_type, account, change, index};
    };
    auto id{seed_id_};
    const auto DeriveKeys =
        [&](auto subchain, auto index, auto& vector) -> bool {
        auto output{true};
        const auto pKey = api_.Crypto().Seed().GetHDKey(
            id, EcdsaCurve::secp256k1, MakePath(subchain, index), reason_);

        EXPECT_TRUE(pKey);

        if (!pKey) { return false; }

        output &= (seed_id_ == id);

        EXPECT_EQ(seed_id_, id);

        const auto& key = *pKey;
        const auto& pubkey = vector.emplace_back(ot::space(key.PublicKey()));
        output &= (false == pubkey.empty());

        EXPECT_FALSE(pubkey.empty());

        return output;
    };

    for (auto i{0u}; i < count_; ++i) {
        EXPECT_TRUE(DeriveKeys(0u, i, external));
        EXPECT_TRUE(DeriveKeys(1u, i, internal));
        EXPECT_NE(external_.at(i), internal_.at(i));
    }
}

TEST_F(Test_BIP44, balance_elements)
{
    const auto test = [&](auto subchain, auto i, const auto& vector) {
        auto output{true};
        const auto next = account_.Reserve(subchain, reason_);

        EXPECT_TRUE(next.has_value());

        if (false == next.has_value()) { return false; }

        EXPECT_EQ(next.value(), i);

        output &= (next.value() == i);
        const auto& element = account_.BalanceElement(subchain, i);
        const auto [account, sub, index] = element.KeyID();
        output &= (account == account_id_);
        output &= (sub == subchain);
        output &= (index == i);

        EXPECT_EQ(account, account_id_);
        EXPECT_EQ(sub, subchain);
        EXPECT_EQ(index, i);

        const auto pPubkey = element.Key();
        const auto pSeckey = element.PrivateKey(reason_);

        EXPECT_TRUE(pPubkey);
        EXPECT_TRUE(pSeckey);

        if ((!pPubkey) || (!pSeckey)) { return false; }

        const auto& pubkey = *pPubkey;
        const auto& seckey = *pSeckey;
        const auto& expected = vector.at(i);
        const auto bytes = ot::reader(expected);
        const auto pubBytes = pubkey.PublicKey();
        const auto secBytes = seckey.PublicKey();

        output &= (pubBytes == bytes);
        output &= (secBytes == bytes);

        if (output) { return output; }

        const auto correct = api_.Factory().Data(bytes);
        const auto fromPublic = api_.Factory().Data(pubBytes);
        const auto fromSecret = api_.Factory().Data(secBytes);

        std::cout << "Failure at row " << std::to_string(i) << '\n';
        EXPECT_EQ(fromPublic->asHex(), correct->asHex());
        EXPECT_EQ(fromSecret->asHex(), correct->asHex());

        return output;
    };

    for (auto i{0u}; i < count_; ++i) {
        using Subchain = ot::blockchain::crypto::Subchain;

        EXPECT_TRUE(test(Subchain::External, i, external_));
        EXPECT_TRUE(test(Subchain::Internal, i, internal_));
    }
}

TEST_F(Test_BIP44, shutdown) { const_cast<ot::Nym_p&>(nym_).reset(); }
}  // namespace ottest
