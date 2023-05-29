// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <functional>
#include <utility>

#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;

using Style = ot::blockchain::crypto::AddressStyle;
using Chain = ot::blockchain::Type;
using TestData = ot::UnallocatedMap<
    ot::UnallocatedCString,
    std::pair<Style, ot::UnallocatedSet<Chain>>>;
using SegwitGood = ot::UnallocatedMap<
    ot::UnallocatedCString,
    std::pair<Chain, ot::UnallocatedCString>>;
using SegwitBad = ot::UnallocatedVector<ot::UnallocatedCString>;

// https://en.bitcoin.it/wiki/Technical_background_of_version_1_Bitcoin_addresses
// https://bitcoin.stackexchange.com/questions/62781/litecoin-constants-and-prefixes

const auto vector_ = TestData{
    {"17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem",
     {Style::P2PKH,
      {Chain::Bitcoin, Chain::BitcoinCash, Chain::BitcoinSV, Chain::eCash}}},
    {"3EktnHQD7RiAE6uzMj2ZifT9YgRrkSgzQX",
     {Style::P2SH,
      {Chain::Bitcoin,
       Chain::BitcoinCash,
       Chain::Litecoin,
       Chain::BitcoinSV,
       Chain::eCash}}},
    {"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn",
     {Style::P2PKH,
      {Chain::Bitcoin_testnet3,
       Chain::BitcoinCash_testnet3,
       Chain::BitcoinCash_testnet4,
       Chain::Litecoin_testnet4,
       Chain::PKT_testnet,
       Chain::BitcoinSV_testnet3,
       Chain::eCash_testnet3,
       Chain::UnitTest}}},
    {"2MzQwSSnBHWHqSAqtTVQ6v47XtaisrJa1Vc",
     {Style::P2SH,
      {Chain::Bitcoin_testnet3,
       Chain::BitcoinCash_testnet3,
       Chain::BitcoinCash_testnet4,
       Chain::Litecoin_testnet4,
       Chain::PKT_testnet,
       Chain::BitcoinSV_testnet3,
       Chain::eCash_testnet3,
       Chain::UnitTest}}},
    {"LM2WMpR1Rp6j3Sa59cMXMs1SPzj9eXpGc1", {Style::P2PKH, {Chain::Litecoin}}},
    {"3MSvaVbVFFLML86rt5eqgA9SvW23upaXdY",
     {Style::P2SH,
      {Chain::Bitcoin,
       Chain::BitcoinCash,
       Chain::Litecoin,
       Chain::BitcoinSV,
       Chain::eCash}}},
    {"MTf4tP1TCNBn8dNkyxeBVoPrFCcVzxJvvh", {Style::P2SH, {Chain::Litecoin}}},
    {"2N2PJEucf6QY2kNFuJ4chQEBoyZWszRQE16",
     {Style::P2SH,
      {Chain::Bitcoin_testnet3,
       Chain::BitcoinCash_testnet3,
       Chain::BitcoinCash_testnet4,
       Chain::Litecoin_testnet4,
       Chain::PKT_testnet,
       Chain::BitcoinSV_testnet3,
       Chain::eCash_testnet3,
       Chain::UnitTest}}},
    {"QVk4MvUu7Wb7tZ1wvAeiUvdF7wxhvpyLLK",
     {Style::P2SH, {Chain::Litecoin_testnet4}}},
    {"pS8EA1pKEVBvv3kGsSGH37R8YViBmuRCPn", {Style::P2PKH, {Chain::PKT}}},
};
const auto p2wpkh_ = SegwitGood{
    {"BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4",
     {Chain::Bitcoin, "751e76e8199196d454941c45d1b3a323f1433bd6"}},
};
const auto p2wsh_ = SegwitGood{
    {"tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sl5k7",
     {Chain::Bitcoin_testnet3,
      "1863143c14c5166804bd19203356da136c985678cd4d27a1b8c6329604903262"}},
    {"tb1qqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesrxh6hy",
     {Chain::Bitcoin_testnet3,
      "000000c4a5cad46221b2a187905e5266362b99d5e91c6ce24d165dab93e86433"}},
};
const auto invalid_segwit_ = SegwitBad{
    "bc1pw508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7k7grpl"
    "x",
    "BC1SW50QA3JX3S",
    "bc1zw508d6qejxtdg4y5r3zarvaryvg6kdaj",
    "tc1qw508d6qejxtdg4y5r3zarvary0c5xw7kg3g4ty",
    "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t5",
    "BC13W508D6QEJXTDG4Y5R3ZARVARY0C5XW7KN40WF2",
    "bc1rw5uspcuh",
    "bc10w508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7kw5rljs"
    "90",
    "BC1QR508D6QEJXTDG4Y5R3ZARVARYV98GJ9P",
    "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sL5k7",
    "bc1zw508d6qejxtdg4y5r3zarvaryvqyzf3du",
    "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3pjxtptv",
    "bc1gmk9yu",
};

namespace ottest
{
class Test_Address : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;

    Test_Address()
        : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    {
    }
};

TEST_F(Test_Address, decode)
{
    for (const auto& [address, data] : vector_) {
        const auto& [expectedStyle, expectedChains] = data;
        const auto [bytes, style, chains, supported] =
            api_.Crypto().Blockchain().DecodeAddress(address);

        EXPECT_EQ(style, expectedStyle);
        EXPECT_EQ(chains.size(), expectedChains.size());
        EXPECT_TRUE(supported);

        for (const auto& chain : expectedChains) {
            EXPECT_EQ(chains.count(chain), 1);
        }
    }
}

TEST_F(Test_Address, segwit)
{
    for (const auto& [address, data] : p2wpkh_) {
        const auto& [chain, payload] = data;
        const auto [bytes, style, chains, supported] =
            api_.Crypto().Blockchain().DecodeAddress(address);
        const auto expected = api_.Factory().DataFromHex(payload);

        EXPECT_EQ(style, Style::P2WPKH);
        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(*chains.begin(), chain);
        EXPECT_EQ(expected, bytes);
        EXPECT_TRUE(supported);
    }

    for (const auto& [address, data] : p2wsh_) {
        const auto& [chain, payload] = data;
        const auto [bytes, style, chains, supported] =
            api_.Crypto().Blockchain().DecodeAddress(address);
        const auto expected = api_.Factory().DataFromHex(payload);

        EXPECT_EQ(style, Style::P2WSH);
        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(*chains.begin(), chain);
        EXPECT_EQ(expected, bytes);
        EXPECT_TRUE(supported);
    }

    for (const auto& address : invalid_segwit_) {
        const auto [bytes, style, chains, supported] =
            api_.Crypto().Blockchain().DecodeAddress(address);

        EXPECT_EQ(bytes.size(), 0);
        EXPECT_EQ(style, Style::Unknown);
        EXPECT_EQ(chains.size(), 0);
        EXPECT_FALSE(supported);
    }
}
}  // namespace ottest
