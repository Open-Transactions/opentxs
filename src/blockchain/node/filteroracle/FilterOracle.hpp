// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Position.hpp"

#pragma once

#include <memory>
#include <string_view>

#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class Position;
}  // namespace block

namespace cfilter
{
class Header;
}  // namespace cfilter

namespace node
{
namespace filteroracle
{
class Shared;
}  // namespace filteroracle

class FilterOracle;
class Manager;
}  // namespace node

class GCS;
}  // namespace blockchain

namespace network
{
namespace otdht
{
class Data;
}  // namespace otdht
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
class FilterOracle final : virtual public node::internal::FilterOracle
{
public:
    auto DefaultType() const noexcept -> cfilter::Type final;
    auto FilterTip(const cfilter::Type type) const noexcept
        -> block::Position final;
    auto GetFilterJob() const noexcept -> CfilterJob final;
    auto GetHeaderJob() const noexcept -> CfheaderJob final;
    auto LoadFilter(
        const cfilter::Type type,
        const block::Hash& block,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> GCS final;
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> Vector<GCS> final;
    auto LoadFilterHeader(const cfilter::Type type, const block::Hash& block)
        const noexcept -> cfilter::Header final;
    auto ProcessBlock(
        const bitcoin::block::Block& block,
        alloc::Default monotonic) const noexcept -> bool final;
    auto ProcessSyncData(
        const block::Hash& prior,
        const Vector<block::Hash>& hashes,
        const network::otdht::Data& data,
        alloc::Default monotonic) const noexcept -> void final;
    auto Tip(const cfilter::Type type) const noexcept -> block::Position final;

    auto Heartbeat() noexcept -> void final;
    auto Init(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void final;
    auto Shutdown() noexcept -> void final;
    auto Start() noexcept -> void final;

    FilterOracle(
        const api::Session& api,
        const node::Manager& node,
        const blockchain::cfilter::Type filter) noexcept;
    FilterOracle() = delete;
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    auto operator=(const FilterOracle&) -> FilterOracle& = delete;
    auto operator=(FilterOracle&&) -> FilterOracle& = delete;

    ~FilterOracle() final;

private:
    mutable std::shared_ptr<filteroracle::Shared> shared_p_;
    filteroracle::Shared& shared_;
};
}  // namespace opentxs::blockchain::node::implementation
