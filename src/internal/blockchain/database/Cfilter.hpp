// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>
#include <tuple>

#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block

namespace cfilter
{
class GCS;
class Hash;
class Header;
}  // namespace cfilter
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Cfilter
{
public:
    using CFHeaderParams =
        std::tuple<block::Hash, cfilter::Header, cfilter::Hash>;
    using CFilterParams = std::pair<block::Hash, cfilter::GCS>;

    virtual auto FilterHeaderTip(const cfilter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto FilterTip(const cfilter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto HaveFilter(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool = 0;
    virtual auto HaveFilterHeader(
        const cfilter::Type type,
        const block::Hash& block) const noexcept -> bool = 0;
    virtual auto LoadFilter(
        const cfilter::Type type,
        const ReadView block,
        alloc::Strategy alloc) const noexcept -> cfilter::GCS = 0;
    virtual auto LoadFilters(
        const cfilter::Type type,
        std::span<const block::Hash> blocks,
        alloc::Strategy alloc) const noexcept -> Vector<cfilter::GCS> = 0;
    virtual auto LoadFilterHash(const cfilter::Type type, const ReadView block)
        const noexcept -> cfilter::Hash = 0;
    virtual auto LoadFilterHeader(
        const cfilter::Type type,
        const ReadView block) const noexcept -> cfilter::Header = 0;

    virtual auto SetFilterHeaderTip(
        const cfilter::Type type,
        const block::Position& position) noexcept -> bool = 0;
    virtual auto SetFilterTip(
        const cfilter::Type type,
        const block::Position& position) noexcept -> bool = 0;
    virtual auto StoreFilters(
        const cfilter::Type type,
        Vector<CFilterParams> filters,
        alloc::Strategy alloc) noexcept -> bool = 0;
    virtual auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        const block::Position& tip,
        alloc::Strategy alloc) noexcept -> bool = 0;
    virtual auto StoreFilterHeaders(
        const cfilter::Type type,
        const ReadView previous,
        const Vector<CFHeaderParams> headers) noexcept -> bool = 0;

    virtual ~Cfilter() = default;
};
}  // namespace opentxs::blockchain::database
