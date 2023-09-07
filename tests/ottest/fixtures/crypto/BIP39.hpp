// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT BIP39 : public ::testing::Test
{
public:
    static constexpr auto type_{ot::crypto::SeedStyle::BIP39};
    static constexpr auto lang_{ot::crypto::Language::en};
    static ot::UnallocatedSet<ot::crypto::SeedID> generated_seeds_;

    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;
    using Languages =
        ot::UnallocatedMap<ot::crypto::Language, ot::UnallocatedCString>;
    using Strengths =
        ot::UnallocatedMap<ot::crypto::SeedStrength, ot::UnallocatedCString>;
    using Types =
        ot::UnallocatedMap<ot::crypto::SeedStyle, ot::UnallocatedCString>;

    static auto expected_seed_languages() noexcept
        -> const ot::UnallocatedMap<ot::crypto::SeedStyle, Languages>&;
    static auto expected_seed_strength() noexcept
        -> const ot::UnallocatedMap<ot::crypto::SeedStyle, Strengths>&;
    static auto expected_seed_types() noexcept -> const Types&;

    static auto word_count(const ot::UnallocatedCString& in) noexcept
        -> std::size_t;

    auto generate_words(const ot::crypto::SeedStrength count) const
        -> std::size_t;

    BIP39();
};
}  // namespace ottest
