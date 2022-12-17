// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
class Header;
}  // namespace cfilter

namespace node
{
namespace internal
{
class FilterOracle;
}  // namespace internal
}  // namespace node

class GCS;
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class OPENTXS_EXPORT FilterOracle
{
public:
    virtual auto DefaultType() const noexcept -> cfilter::Type = 0;
    virtual auto FilterTip(const cfilter::Type type) const noexcept
        -> block::Position = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::FilterOracle& = 0;
    virtual auto LoadFilter(
        const cfilter::Type type,
        const block::Hash& block,
        alloc::Default alloc,
        alloc::Default monotonic = {}) const noexcept -> GCS = 0;
    virtual auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks,
        alloc::Default alloc,
        alloc::Default monotonic = {}) const noexcept -> Vector<GCS> = 0;
    virtual auto LoadFilterHeader(
        const cfilter::Type type,
        const block::Hash& block) const noexcept -> cfilter::Header = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::FilterOracle& = 0;

    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    auto operator=(const FilterOracle&) -> FilterOracle& = delete;
    auto operator=(FilterOracle&&) -> FilterOracle& = delete;

    virtual ~FilterOracle() = default;

protected:
    FilterOracle() noexcept = default;
};
}  // namespace opentxs::blockchain::node
