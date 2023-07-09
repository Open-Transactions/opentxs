// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string_view>

#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace node
{
namespace wallet
{
class ReorgMasterPrivate;
class ReorgSlave;
}  // namespace wallet

class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Pipeline;
}  // namespace zeromq
}  // namespace network

namespace storage
{
namespace lmdb
{
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class ReorgMaster final : public Allocated, public Reorg
{
public:
    auto get_allocator() const noexcept -> allocator_type final;

    auto CheckShutdown() noexcept -> bool;
    auto ClearReorg() noexcept -> void;
    auto FinishReorg() noexcept -> void;
    [[nodiscard]] auto GetReorg(
        const block::Position& position,
        storage::lmdb::Transaction&& tx) noexcept -> Params&;
    [[nodiscard]] auto GetSlave(
        const network::zeromq::Pipeline& parent,
        std::string_view name,
        alloc::Strategy alloc) noexcept -> ReorgSlave final;
    [[nodiscard]] auto PerformReorg(const node::HeaderOracle& oracle) noexcept
        -> bool;
    [[nodiscard]] auto PrepareReorg(StateSequence id) noexcept -> bool;
    [[nodiscard]] auto PrepareShutdown() noexcept -> bool;
    auto Stop() noexcept -> void;

    ReorgMaster(
        const network::zeromq::Pipeline& parent,
        allocator_type alloc) noexcept;
    ReorgMaster(const ReorgMaster&) = delete;
    ReorgMaster(ReorgMaster&&) = delete;
    auto operator=(const ReorgMaster&) -> ReorgMaster& = delete;
    auto operator=(ReorgMaster&&) -> ReorgMaster& = delete;

    ~ReorgMaster() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<ReorgMasterPrivate> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
