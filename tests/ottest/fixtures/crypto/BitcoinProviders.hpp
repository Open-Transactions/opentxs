// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT Bitcoin_Providers : public ::testing::Test
{
public:
    static const bool have_hd_;

    const ot::api::session::Client& client_;
    ot::PasswordPrompt reason_;
    const ot::api::Crypto& crypto_;
    const ot::UnallocatedMap<ot::UnallocatedCString, ot::UnallocatedCString>
        base_58_;
    const ot::UnallocatedMap<ot::UnallocatedCString, ot::UnallocatedCString>
        ripemd160_;

    using Path = ot::UnallocatedVector<std::uint32_t>;
    using Bip32TestCase =
        std::tuple<Path, ot::UnallocatedCString, ot::UnallocatedCString>;
    using Bip32TestVector =
        std::pair<ot::UnallocatedCString, ot::UnallocatedVector<Bip32TestCase>>;

    const ot::UnallocatedVector<Bip32TestVector> bip_32_;
    const ot::UnallocatedMap<
        ot::UnallocatedCString,
        std::tuple<
            ot::UnallocatedCString,
            ot::UnallocatedCString,
            ot::UnallocatedCString>>
        bip_39_;

    Bitcoin_Providers();

    auto test_base58_encode() -> bool;
    auto test_base58_decode() -> bool;
    auto test_ripemd160() -> bool;
    auto get_seed(const ot::UnallocatedCString& hex) const -> ot::Secret;
    auto test_bip32_seed(const ot::crypto::Bip32& library) -> bool;
    auto compare_private(
        const ot::crypto::Bip32& library,
        const std::string_view lhs,
        const std::string_view rhs) const -> bool;
    auto compare_public(
        const ot::crypto::Bip32& library,
        const std::string_view lhs,
        const std::string_view rhs) const -> bool;
    auto test_bip32_child_key(const ot::crypto::Bip32& library) -> bool;
    auto test_bip39(const ot::crypto::Bip32& library) -> bool;
};
}  // namespace ottest
