// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/cfilter/GCS.hpp"

namespace opentxs::factory
{
auto GCS(
    const api::Session&,
    const std::uint8_t,
    const std::uint32_t,
    const ReadView,
    const Vector<ByteArray>&,
    alloc::Default) noexcept -> blockchain::cfilter::GCS
{
    return {};
}

auto GCS(const api::Session&, const protobuf::GCS&, alloc::Default) noexcept
    -> blockchain::cfilter::GCS
{
    return {};
}

auto GCS(const api::Session&, const ReadView, alloc::Default) noexcept
    -> blockchain::cfilter::GCS
{
    return {};
}

auto GCS(
    const api::Session&,
    const std::uint8_t,
    const std::uint32_t,
    const ReadView,
    const std::uint32_t,
    const ReadView,
    alloc::Default) noexcept -> blockchain::cfilter::GCS
{
    return {};
}

auto GCS(
    const api::Session&,
    const blockchain::cfilter::Type,
    const ReadView,
    const ReadView,
    alloc::Default) noexcept -> blockchain::cfilter::GCS
{
    return {};
}

auto GCS(
    const api::Session&,
    const blockchain::cfilter::Type type,
    const blockchain::block::Block& block,
    alloc::Default,
    alloc::Default) noexcept -> blockchain::cfilter::GCS
{
    return {};
}
}  // namespace opentxs::factory
