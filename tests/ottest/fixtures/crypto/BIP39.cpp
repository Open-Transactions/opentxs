// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/crypto/BIP39.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cctype>
#include <cstddef>
#include <functional>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;

ot::UnallocatedSet<ot::crypto::SeedID> BIP39::generated_seeds_{};

auto BIP39::expected_seed_languages() noexcept
    -> const ot::UnallocatedMap<ot::crypto::SeedStyle, Languages>&
{
    using Language = ot::crypto::Language;
    static const auto data =
        ot::UnallocatedMap<ot::crypto::SeedStyle, Languages>{
            {ot::crypto::SeedStyle::BIP39,
             {
                 {Language::en, "English"},
             }},
            {ot::crypto::SeedStyle::PKT,
             {
                 {Language::en, "English"},
             }},
        };

    return data;
}
auto BIP39::expected_seed_strength() noexcept
    -> const ot::UnallocatedMap<ot::crypto::SeedStyle, Strengths>&
{
    using Style = ot::crypto::SeedStyle;
    using Strength = ot::crypto::SeedStrength;
    static const auto data =
        ot::UnallocatedMap<ot::crypto::SeedStyle, Strengths>{
            {Style::BIP39,
             {
                 {Strength::Twelve, "12 words"},
                 {Strength::Fifteen, "15 words"},
                 {Strength::Eighteen, "18 words"},
                 {Strength::TwentyOne, "21 words"},
                 {Strength::TwentyFour, "24 words"},
             }},
            {Style::PKT,
             {
                 {Strength::Fifteen, "15 words"},
             }},
        };

    return data;
}
auto BIP39::expected_seed_types() noexcept -> const Types&
{
    static const auto data = Types{
        {ot::crypto::SeedStyle::BIP39, "BIP-39"},
        {ot::crypto::SeedStyle::PKT, "Legacy pktwallet"},
    };

    return data;
}

auto BIP39::word_count(const ot::UnallocatedCString& in) noexcept -> std::size_t
{
    if (0 == in.size()) { return 0; }

    auto word = false;
    using namespace opentxs::literals;
    using namespace std::literals;
    auto count = 0_uz;

    for (const auto c : in) {
        if (std::isspace(c)) {
            if (word) {
                word = false;
            } else {
                continue;
            }
        } else {
            if (word) {
                continue;
            } else {
                word = true;
                ++count;
            }
        }
    }

    return count;
}

auto BIP39::generate_words(const ot::crypto::SeedStrength count) const
    -> std::size_t
{
    const auto fingerprint =
        api_.Crypto().Seed().NewSeed(type_, lang_, count, reason_);

    EXPECT_EQ(generated_seeds_.count(fingerprint), 0);

    if (generated_seeds_.contains(fingerprint)) { return 0; }

    generated_seeds_.insert(fingerprint);

    const auto words = api_.Crypto().Seed().Words(fingerprint, reason_);

    return word_count(words);
}

BIP39::BIP39()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
{
}
}  // namespace ottest
