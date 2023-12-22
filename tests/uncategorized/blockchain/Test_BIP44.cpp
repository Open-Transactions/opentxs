// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <iostream>
#include <string_view>

#include "internal/util/DeferredConstruction.hpp"
#include "ottest/fixtures/blockchain/BIP44.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

using Pubkey = ot::Space;
using ExpectedKeys = ot::UnallocatedVector<Pubkey>;
const ExpectedKeys external_{};
const ExpectedKeys internal_{};

TEST_F(BIP44, init)
{
    EXPECT_FALSE(seed_id_.get().empty());
    EXPECT_FALSE(nym_id_.empty());
    EXPECT_EQ(account_.ID(), account_id_);
    EXPECT_EQ(account_.Standard(), ot::blockchain::crypto::HDProtocol::BIP_44);
}

TEST_F(BIP44, generate_expected_keys)
{
    auto& internal = const_cast<ExpectedKeys&>(internal_);
    auto& external = const_cast<ExpectedKeys&>(external_);
    internal.reserve(count_);
    external.reserve(count_);
    using Path = ot::UnallocatedVector<ot::Bip32Index>;
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
    auto id{seed_id_.get()};
    const auto DeriveKeys =
        [&](auto subchain, auto index, auto& vector) -> bool {
        auto output{true};
        const auto key = api_.Crypto().Seed().GetHDKey(
            id,
            ot::crypto::EcdsaCurve::secp256k1,
            MakePath(subchain, index),
            reason_);

        EXPECT_TRUE(key.IsValid());

        if (false == key.IsValid()) { return false; }

        output &= (seed_id_.get() == id);

        EXPECT_EQ(seed_id_.get(), id);

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

TEST_F(BIP44, balance_elements)
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

        const auto& pubkey = element.Key();
        const auto& seckey = element.PrivateKey(reason_);

        EXPECT_TRUE(pubkey.IsValid());
        EXPECT_TRUE(seckey.IsValid());

        output &= pubkey.IsValid();
        output &= seckey.IsValid();
        const auto& expected = vector.at(i);
        const auto bytes = ot::reader(expected);
        const auto pubBytes = pubkey.PublicKey();
        const auto secBytes = seckey.PublicKey();

        output &= (pubBytes == bytes);
        output &= (secBytes == bytes);

        if (output) { return output; }

        const auto correct = api_.Factory().DataFromBytes(bytes);
        const auto fromPublic = api_.Factory().DataFromBytes(pubBytes);
        const auto fromSecret = api_.Factory().DataFromBytes(secBytes);

        std::cout << "Failure at row " << std::to_string(i) << '\n';
        EXPECT_EQ(fromPublic.asHex(), correct.asHex());
        EXPECT_EQ(fromSecret.asHex(), correct.asHex());

        return output;
    };

    for (auto i{0u}; i < count_; ++i) {
        using Subchain = ot::blockchain::crypto::Subchain;

        EXPECT_TRUE(test(Subchain::External, i, external_));
        EXPECT_TRUE(test(Subchain::Internal, i, internal_));
    }
}
}  // namespace ottest
