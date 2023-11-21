// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/cfilter/Header.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Position.hpp"

#pragma once

#include <memory>

#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
class Hash;
}  // namespace block

namespace database
{
class Cfilter;
}  // namespace database

namespace node
{
namespace filteroracle
{
class Shared;
}  // namespace filteroracle

namespace internal
{
struct Config;
}  // namespace internal

class HeaderOracle;
class Manager;
struct Endpoints;
}  // namespace node
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
    auto LoadFilter(
        const cfilter::Type type,
        const block::Hash& block,
        alloc::Strategy alloc) const noexcept -> cfilter::GCS final;
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks,
        alloc::Strategy alloc) const noexcept -> Vector<cfilter::GCS> final;
    auto LoadFilterHeader(const cfilter::Type type, const block::Hash& block)
        const noexcept -> cfilter::Header final;
    auto ProcessBlock(const block::Block& block, alloc::Default monotonic)
        const noexcept -> bool final;
    auto ProcessSyncData(
        const block::Hash& prior,
        const Vector<block::Hash>& hashes,
        const network::otdht::Data& data,
        alloc::Default monotonic) const noexcept -> void final;
    auto Tip(const cfilter::Type type) const noexcept -> block::Position final;

    auto Heartbeat() noexcept -> void final;
    auto Init(
        std::shared_ptr<const api::internal::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void final;

    FilterOracle(
        const api::Session& api,
        const node::HeaderOracle& header,
        const node::Endpoints& endpoints,
        const node::internal::Config& config,
        database::Cfilter& db,
        blockchain::Type chain,
        blockchain::cfilter::Type filter) noexcept;
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
