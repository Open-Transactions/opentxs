// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <functional>
#include <span>
#include <string_view>
#include <utility>

#include "ottest/data/blockchain/Address.hpp"
#include "ottest/data/crypto/Hashes.hpp"
#include "ottest/fixtures/common/OneClientSession.hpp"

namespace ottest
{
using enum opentxs::blockchain::crypto::AddressStyle;

TEST_F(OneClientSession, base58)
{
    for (const auto& [address, data] : Base58Addresses()) {
        const auto& [expectedStyle, expectedChains] = data;
        const auto [bytes, style, chains, supported] =
            client_1_.Crypto().Blockchain().DecodeAddress(address);

        EXPECT_EQ(style, expectedStyle);
        EXPECT_EQ(chains.size(), expectedChains.size());
        EXPECT_TRUE(supported);

        for (const auto& chain : expectedChains) {
            EXPECT_EQ(chains.count(chain), 1);
        }
    }
}

TEST_F(OneClientSession, ethereum)
{
    using enum opentxs::blockchain::Type;
    using enum opentxs::blockchain::crypto::AddressStyle;
    using enum opentxs::crypto::asymmetric::Algorithm;
    using opentxs::ByteArray;

    const auto& blockchain = client_1_.Crypto().Blockchain();
    const auto& factory = client_1_.Factory();
    const auto reason = factory.PasswordPrompt(__func__);

    for (const auto& [sec, pub, pkh, last20, cs, addr] : EthHashes()) {
        const auto decoded = ByteArray{opentxs::IsHex, sec};
        const auto key = factory.AsymmetricKey(
            Secp256k1, factory.SecretFromBytes(decoded.Bytes()), reason, {});
        const auto [bytes, style, chains, supported] =
            blockchain.DecodeAddress(addr);

        EXPECT_TRUE(supported);
        EXPECT_EQ(style, ethereum_account);
        EXPECT_EQ(last20, bytes.asHex());
        EXPECT_TRUE(key.IsValid());
        EXPECT_EQ(key.Type(), Secp256k1);

        {
            const auto encoded = blockchain.EncodeAddress(
                ethereum_account, Ethereum, key.asEllipticCurve());
            auto view = std::string_view{encoded};

            ASSERT_EQ(view.size(), 42);

            view.remove_prefix(2);

            EXPECT_EQ(view, addr);
        }

        {
            const auto encoded = blockchain.EncodeAddress(
                ethereum_account, Ethereum, ByteArray{opentxs::IsHex, last20});
            auto view = std::string_view{encoded};

            ASSERT_EQ(view.size(), 42);

            view.remove_prefix(2);

            EXPECT_EQ(view, addr);
        }
    }
}

TEST_F(OneClientSession, segwit)
{
    for (const auto& [address, data] : SegwitP2WPKH()) {
        const auto& [chain, payload] = data;
        const auto [bytes, style, chains, supported] =
            client_1_.Crypto().Blockchain().DecodeAddress(address);
        const auto expected = client_1_.Factory().DataFromHex(payload);

        EXPECT_EQ(style, p2wpkh);
        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(*chains.begin(), chain);
        EXPECT_EQ(expected, bytes);
        EXPECT_TRUE(supported);
    }

    for (const auto& [address, data] : SegwitP2WSH()) {
        const auto& [chain, payload] = data;
        const auto [bytes, style, chains, supported] =
            client_1_.Crypto().Blockchain().DecodeAddress(address);
        const auto expected = client_1_.Factory().DataFromHex(payload);

        EXPECT_EQ(style, p2wsh);
        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(*chains.begin(), chain);
        EXPECT_EQ(expected, bytes);
        EXPECT_TRUE(supported);
    }

    for (const auto& address : SegwitInvalid()) {
        const auto [bytes, style, chains, supported] =
            client_1_.Crypto().Blockchain().DecodeAddress(address);

        EXPECT_EQ(bytes.size(), 0);
        EXPECT_EQ(style, unknown_address_style);
        EXPECT_EQ(chains.size(), 0);
        EXPECT_FALSE(supported);
    }
}
}  // namespace ottest
