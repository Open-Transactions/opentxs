// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <span>

namespace ottest
{
namespace ot = opentxs;

struct OPENTXS_EXPORT Bip158Vector {
    ot::blockchain::block::Height height_;
    ot::UnallocatedCString block_hash_;
    ot::UnallocatedCString block_;
    ot::UnallocatedVector<ot::UnallocatedCString> previous_outputs_;
    ot::UnallocatedCString previous_filter_header_;
    ot::UnallocatedCString filter_;
    ot::UnallocatedCString filter_header_;
    ot::UnallocatedCString notes_;

    auto Block(const ot::api::Session& api) const noexcept -> ot::ByteArray;
    auto BlockHash(const ot::api::Session& api) const noexcept -> ot::ByteArray;
    auto Filter(const ot::api::Session& api) const noexcept -> ot::ByteArray;
    auto FilterHeader(const ot::api::Session& api) const noexcept
        -> ot::ByteArray;
    auto PreviousFilterHeader(const ot::api::Session& api) const noexcept
        -> ot::ByteArray;
    auto PreviousOutputs(const ot::api::Session& api) const noexcept
        -> ot::UnallocatedVector<ot::ByteArray>;
};

auto OPENTXS_EXPORT GetBchCfilter1307544() noexcept
    -> std::span<const std::byte>;
auto OPENTXS_EXPORT GetBchCfilter1307723() noexcept
    -> std::span<const std::byte>;
auto OPENTXS_EXPORT GetBip158Elements() noexcept -> const ot::UnallocatedMap<
    ot::blockchain::block::Height,
    ot::UnallocatedVector<ot::UnallocatedCString>>&;
auto OPENTXS_EXPORT GetBip158Vectors() noexcept
    -> const ot::UnallocatedVector<Bip158Vector>&;
auto OPENTXS_EXPORT some_moron_wrote_the_bytes_backwards(
    const ot::UnallocatedCString& in) -> ot::UnallocatedCString;
auto OPENTXS_EXPORT parse_hex(
    const ot::api::Session& api,
    const ot::UnallocatedCString& hex,
    const bool reverse = false) noexcept -> ot::ByteArray;
}  // namespace ottest
