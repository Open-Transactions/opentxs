// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/data/blockchain/Address.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/opentxs.hpp"

namespace ottest
{
using namespace std::literals;

auto Base58Addresses() noexcept -> const Base58Address&
{
    // https://en.bitcoin.it/wiki/Technical_background_of_version_1_Bitcoin_addresses
    // https://bitcoin.stackexchange.com/questions/62781/litecoin-constants-and-prefixes
    using enum opentxs::blockchain::Type;
    using enum opentxs::blockchain::crypto::AddressStyle;

    static const auto data = Base58Address{
        {"17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem"sv,
         {p2pkh, {Bitcoin, BitcoinCash, BitcoinSV, eCash}}},
        {"3EktnHQD7RiAE6uzMj2ZifT9YgRrkSgzQX"sv,
         {p2sh, {Bitcoin, BitcoinCash, Litecoin, BitcoinSV, eCash}}},
        {"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn"sv,
         {p2pkh,
          {Bitcoin_testnet3,
           BitcoinCash_testnet3,
           BitcoinCash_testnet4,
           Litecoin_testnet4,
           PKT_testnet,
           BitcoinSV_testnet3,
           eCash_testnet3,
           UnitTest}}},
        {"2MzQwSSnBHWHqSAqtTVQ6v47XtaisrJa1Vc"sv,
         {p2sh,
          {Bitcoin_testnet3,
           BitcoinCash_testnet3,
           BitcoinCash_testnet4,
           Litecoin_testnet4,
           PKT_testnet,
           BitcoinSV_testnet3,
           eCash_testnet3,
           UnitTest}}},
        {"LM2WMpR1Rp6j3Sa59cMXMs1SPzj9eXpGc1"sv, {p2pkh, {Litecoin}}},
        {"3MSvaVbVFFLML86rt5eqgA9SvW23upaXdY"sv,
         {p2sh, {Bitcoin, BitcoinCash, Litecoin, BitcoinSV, eCash}}},
        {"MTf4tP1TCNBn8dNkyxeBVoPrFCcVzxJvvh"sv, {p2sh, {Litecoin}}},
        {"2N2PJEucf6QY2kNFuJ4chQEBoyZWszRQE16"sv,
         {p2sh,
          {Bitcoin_testnet3,
           BitcoinCash_testnet3,
           BitcoinCash_testnet4,
           Litecoin_testnet4,
           PKT_testnet,
           BitcoinSV_testnet3,
           eCash_testnet3,
           UnitTest}}},
        {"QVk4MvUu7Wb7tZ1wvAeiUvdF7wxhvpyLLK"sv, {p2sh, {Litecoin_testnet4}}},
        {"pS8EA1pKEVBvv3kGsSGH37R8YViBmuRCPn"sv, {p2pkh, {PKT}}},
        {"XpESxaUmonkq8RaLLp46Brx2K39ggQe226"sv, {p2pkh, {Dash}}},
        {"XyHHinPZB5Q4FC9jtbksaQBJmXq48gtVGb"sv, {p2pkh, {Dash}}},
        {"7fePc8Mf7RYghdxkSp5yaWfT9WR4F2Dsqa"sv, {p2sh, {Dash}}},
        {"yNGZtDY3Qt8JioTpWAFDcKTxEiC6oVv2Gm"sv, {p2pkh, {Dash_testnet3}}},
    };

    return data;
}

auto SegwitInvalid() noexcept -> std::span<const std::string_view>
{
    static constexpr auto data = {
        "bc1pw508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7k7g"
        "rpl"
        "x"sv,
        "BC1SW50QA3JX3S"sv,
        "bc1zw508d6qejxtdg4y5r3zarvaryvg6kdaj"sv,
        "tc1qw508d6qejxtdg4y5r3zarvary0c5xw7kg3g4ty"sv,
        "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t5"sv,
        "BC13W508D6QEJXTDG4Y5R3ZARVARY0C5XW7KN40WF2"sv,
        "bc1rw5uspcuh"sv,
        "bc10w508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7kw5"
        "rljs"
        "90"sv,
        "BC1QR508D6QEJXTDG4Y5R3ZARVARYV98GJ9P"sv,
        "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sL5k7"sv,
        "bc1zw508d6qejxtdg4y5r3zarvaryvqyzf3du"sv,
        "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3pjxtptv"sv,
        "bc1gmk9yu"sv,
    };

    return data;
}

auto SegwitP2WPKH() noexcept -> const SegwitAddress&
{
    using enum opentxs::blockchain::Type;

    static const auto data = SegwitAddress{
        {"BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4"sv,
         {Bitcoin, "751e76e8199196d454941c45d1b3a323f1433bd6"sv}},
    };

    return data;
}

auto SegwitP2WSH() noexcept -> const SegwitAddress&
{
    using enum opentxs::blockchain::Type;

    static const auto data = SegwitAddress{
        {"tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sl5k7"sv,
         {Bitcoin_testnet3,
          "1863143c14c5166804bd19203356da136c985678cd4d27a1b8c6329604903262"sv}},
        {"tb1qqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesrxh6hy"sv,
         {Bitcoin_testnet3,
          "000000c4a5cad46221b2a187905e5266362b99d5e91c6ce24d165dab93e86433"sv}},
    };

    return data;
}
}  // namespace ottest
