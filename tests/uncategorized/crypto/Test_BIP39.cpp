// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "crypto/Bip39.hpp"
#include "ottest/fixtures/crypto/BIP39.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(BIP39, seed_types)
{
    const auto& expected = expected_seed_types();
    const auto test = api_.Crypto().Seed().AllowedSeedTypes();

    EXPECT_EQ(test.size(), expected.size());

    for (const auto& [type, text] : expected) {
        try {
            EXPECT_EQ(text, test.at(type));
        } catch (...) {
            EXPECT_TRUE(false);
        }
    }
}

TEST_F(BIP39, seed_languages)
{
    for (const auto& [type, expected] : expected_seed_languages()) {
        const auto test = api_.Crypto().Seed().AllowedLanguages(type);

        EXPECT_EQ(test.size(), expected.size());

        for (const auto& [lang, text] : expected) {
            try {
                EXPECT_EQ(text, test.at(lang));
            } catch (...) {
                EXPECT_TRUE(false);
            }
        }
    }
}

TEST_F(BIP39, seed_strength)
{
    for (const auto& [type, expected] : expected_seed_strength()) {
        const auto test = api_.Crypto().Seed().AllowedSeedStrength(type);

        EXPECT_EQ(test.size(), expected.size());

        for (const auto& [strength, text] : expected) {
            try {
                EXPECT_EQ(text, test.at(strength));
            } catch (...) {
                EXPECT_TRUE(false);
            }
        }
    }
}

TEST_F(BIP39, word_count)
{
    static const auto vector =
        ot::UnallocatedMap<ot::UnallocatedCString, std::size_t>{
            {"", 0},
            {" ", 0},
            {"     ", 0},
            {"one", 1},
            {" one", 1},
            {"   one", 1},
            {"one ", 1},
            {" one ", 1},
            {"   one   ", 1},
            {"one two", 2},
            {" one  two ", 2},
            {"   one   two ", 2},
            {"   one   two 3", 3},
        };

    for (const auto& [string, count] : vector) {
        EXPECT_EQ(word_count(string), count);
    }
}

TEST_F(BIP39, longest_en)
{
    EXPECT_EQ(api_.Crypto().Seed().LongestWord(type_, lang_), 8);
}

TEST_F(BIP39, twelve_words)
{
    const auto& api = api_.Crypto().Seed();
    constexpr auto strength{ot::crypto::SeedStrength::Twelve};
    constexpr auto words{12};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(BIP39, fifteen_words)
{
    const auto& api = api_.Crypto().Seed();
    constexpr auto strength{ot::crypto::SeedStrength::Fifteen};
    constexpr auto words{15};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(BIP39, eighteen_words)
{
    const auto& api = api_.Crypto().Seed();
    constexpr auto strength{ot::crypto::SeedStrength::Eighteen};
    constexpr auto words{18};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(BIP39, twentyone_words)
{
    const auto& api = api_.Crypto().Seed();
    constexpr auto strength{ot::crypto::SeedStrength::TwentyOne};
    constexpr auto words{21};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(BIP39, twentyfour_words)
{
    const auto& api = api_.Crypto().Seed();
    constexpr auto strength{ot::crypto::SeedStrength::TwentyFour};
    constexpr auto words{24};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(BIP39, match_a)
{
    const auto test = ot::UnallocatedCString{"a"};
    const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{
        "abandon", "ability",  "able",     "about",    "above",    "absent",
        "absorb",  "abstract", "absurd",   "abuse",    "access",   "accident",
        "account", "accuse",   "achieve",  "acid",     "acoustic", "acquire",
        "across",  "act",      "action",   "actor",    "actress",  "actual",
        "adapt",   "add",      "addict",   "address",  "adjust",   "admit",
        "adult",   "advance",  "advice",   "aerobic",  "affair",   "afford",
        "afraid",  "again",    "age",      "agent",    "agree",    "ahead",
        "aim",     "air",      "airport",  "aisle",    "alarm",    "album",
        "alcohol", "alert",    "alien",    "all",      "alley",    "allow",
        "almost",  "alone",    "alpha",    "already",  "also",     "alter",
        "always",  "amateur",  "amazing",  "among",    "amount",   "amused",
        "analyst", "anchor",   "ancient",  "anger",    "angle",    "angry",
        "animal",  "ankle",    "announce", "annual",   "another",  "answer",
        "antenna", "antique",  "anxiety",  "any",      "apart",    "apology",
        "appear",  "apple",    "approve",  "april",    "arch",     "arctic",
        "area",    "arena",    "argue",    "arm",      "armed",    "armor",
        "army",    "around",   "arrange",  "arrest",   "arrive",   "arrow",
        "art",     "artefact", "artist",   "artwork",  "ask",      "aspect",
        "assault", "asset",    "assist",   "assume",   "asthma",   "athlete",
        "atom",    "attack",   "attend",   "attitude", "attract",  "auction",
        "audit",   "august",   "aunt",     "author",   "auto",     "autumn",
        "average", "avocado",  "avoid",    "awake",    "aware",    "away",
        "awesome", "awful",    "awkward",  "axis",
    };
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, match_ar)
{
    const auto test = ot::UnallocatedCString{"ar"};
    const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{
        "arch",
        "arctic",
        "area",
        "arena",
        "argue",
        "arm",
        "armed",
        "armor",
        "army",
        "around",
        "arrange",
        "arrest",
        "arrive",
        "arrow",
        "art",
        "artefact",
        "artist",
        "artwork"};
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, match_arr)
{
    const auto test = ot::UnallocatedCString{"arr"};
    const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{
        "arrange", "arrest", "arrive", "arrow"};
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, match_arri)
{
    const auto test = ot::UnallocatedCString{"arri"};
    const auto expected =
        ot::UnallocatedVector<ot::UnallocatedCString>{"arrive"};
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, match_arrive)
{
    const auto test = ot::UnallocatedCString{"arrive"};
    const auto expected =
        ot::UnallocatedVector<ot::UnallocatedCString>{"arrive"};
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, match_arrived)
{
    const auto test = ot::UnallocatedCString{"arrived"};
    const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{};
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, match_axe)
{
    const auto test = ot::UnallocatedCString{"axe"};
    const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{};
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, match_empty_string)
{
    const auto test = ot::UnallocatedCString{""};
    const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{};
    const auto suggestions =
        api_.Crypto().Seed().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(BIP39, validate_en_list)
{
    using Bip39 = opentxs::crypto::implementation::Bip39;

    for (const auto* word : Bip39::words_.at(lang_)) {
        const auto matches =
            api_.Crypto().Seed().ValidateWord(type_, lang_, word);

        EXPECT_EQ(matches.size(), 1);
    }
}

TEST_F(BIP39, pkt_seed_import)
{
    static constexpr auto phrase{
        "forum school old approve bubble warfare robust figure pact glance "
        "farm leg taxi sing ankle"};
    static constexpr auto password{"Password123#"};
    static constexpr auto expected_seed_bytes_{
        "498d7c2713178cb9c78ac00061b0969429792f"};
    static const auto expected_secret_keys_ =
        ot::UnallocatedVector<ot::UnallocatedCString>{
            "885e744b15e847044bbb33c80c8aeb26abb7a2c2a5120b0a64dec8c12062f3a6",
            "0e9bfa981927a86d2a1329984c5a45a3fa8a0a684c351e1970380d6a2d5fecc1",
            "63e72656faf5f756d82430f9f45e273abc24d4303d11ba5cca78ac49f8f1a73d",
            "d2e890b1e46695e27f1bcb32546f71c11f384d485216034ffdadc54516ba3487",
            "427c65d4380bae5c535cedfa4e080c9b61fad98a153ca2572d7459ee3db78b8d",
            "d8b037cea44b93f9fae44b70bf6a210bd3e8fc4718d3a162b9118a751e9bc347",
            "4536717d7da8f4ac0b21b5cba8e9967f36e1b7b3fa852e7c9bd96ad86fc86552",
            "29c7226f31458c2c0192bdd0837ed26be513d20f7606dc763b4d7cd68391a92e",
            "0ba52306295a4af4548aeea16b2d3422a14f81fefc3a08b77a267e2d4296e83b",
            "ea0ef4bd3597ab55cec5243180d65b9272014241ea6e2a7c895feec147ddef92",
        };
    const auto seed = api_.Crypto().Seed().ImportSeed(
        api_.Factory().SecretFromText(phrase),
        api_.Factory().SecretFromText(password),
        ot::crypto::SeedStyle::PKT,
        ot::crypto::Language::en,
        reason_);

    ASSERT_FALSE(seed.empty());

    const auto entropy = api_.Crypto().Seed().Bip32Root(seed, reason_);

    EXPECT_EQ(entropy, expected_seed_bytes_);

    static constexpr auto index{0u};
    const auto pNym =
        api_.Wallet().Nym({api_.Factory(), seed, index}, reason_, "pkt");

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        nym.ID(),
        ot::blockchain::crypto::HDProtocol::BIP_84,
        ot::blockchain::Type::Bitcoin,
        ot::blockchain::Type::PKT,
        reason_);

    ASSERT_FALSE(accountID.empty());

    const auto& account =
        api_.Crypto().Blockchain().HDSubaccount(nym.ID(), accountID);

    EXPECT_EQ(account.Standard(), ot::blockchain::crypto::HDProtocol::BIP_84);

    static constexpr auto subchain{ot::blockchain::crypto::Subchain::External};

    while (*account.LastGenerated(subchain) < 10) {
        account.GenerateNext(subchain, reason_);
    }

    using Index = ot::Bip32Index;

    for (auto i = Index{0}; i < 10u; ++i) {
        const auto& element = account.BalanceElement(subchain, i);
        const auto key = element.PrivateKey(reason_);

        ASSERT_TRUE(key.IsValid());

        const auto secret = [&] {
            auto out = api_.Factory().DataFromBytes(key.PrivateKey(reason_));

            return out;
        }();

        EXPECT_EQ(secret.asHex(), expected_secret_keys_.at(i));
    }
}
}  // namespace ottest
