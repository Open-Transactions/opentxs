// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <string_view>
#include <utility>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/Envelope.hpp"

namespace ottest
{
namespace ot = opentxs;
using namespace opentxs::literals;
using namespace std::literals;

class OPENTXS_EXPORT Envelope : public ::testing::Test
{
public:
    using Nyms = ot::UnallocatedVector<ot::Nym_p>;
    using Test = std::pair<bool, ot::UnallocatedVector<int>>;
    using Expected = ot::UnallocatedVector<Test>;

    static const bool have_rsa_;
    static const bool have_secp256k1_;
    static const bool have_ed25519_;
    static const Expected expected_;
    static Nyms nyms_;

    static bool init_;

    const ot::api::Session& sender_;
    const ot::api::Session& recipient_;
    const ot::PasswordPrompt reason_s_;
    const ot::PasswordPrompt reason_r_;
    const ot::OTString plaintext_;

    static auto can_seal(const std::size_t row) -> bool;
    static auto can_open(const std::size_t row, const std::size_t column)
        -> bool;
    static auto get_armored(const ot::api::Session& api) noexcept
        -> opentxs::OTArmored;
    static auto get_envelope(const ot::api::Session& api) noexcept
        -> opentxs::OTEnvelope;
    static auto get_envelope(
        const ot::api::Session& api,
        const opentxs::Armored& ciphertext) noexcept(false)
        -> opentxs::OTEnvelope;
    static auto get_envelope(
        const ot::api::Session& api,
        const opentxs::ReadView& ciphertext) noexcept(false)
        -> opentxs::OTEnvelope;
    static auto is_active(const std::size_t row, const std::size_t column)
        -> bool;
    static auto should_seal(const std::size_t row, const std::size_t column)
        -> bool;

    Envelope();
};
}  // namespace ottest
