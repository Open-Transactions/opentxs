// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/wallet/subchain/statemachine/Rescan.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <optional>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

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
namespace internal
{
struct HeaderOraclePrivate;
}  // namespace internal

namespace wallet
{
class SubchainStateData;
}  // namespace wallet

class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Rescan::Imp final : public statemachine::Job
{
public:
    Imp(const boost::shared_ptr<const SubchainStateData>& parent,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final = default;

private:
    network::zeromq::socket::Raw& to_process_;
    network::zeromq::socket::Raw& to_progress_;
    std::optional<block::Position> last_scanned_;
    std::optional<block::Position> filter_tip_;
    block::Position highest_dirty_;
    Set<block::Position> dirty_;

    auto before(const block::Position& position) const noexcept
        -> block::Position;
    auto can_advance() const noexcept -> bool;
    auto caught_up() const noexcept -> bool;
    auto current() const noexcept -> const block::Position&;
    auto highest_clean(const Set<block::Position>& clean) const noexcept
        -> std::optional<block::Position>;
    auto rescan_finished() const noexcept -> bool;
    auto stop() const noexcept -> block::Height;

    auto adjust_last_scanned(
        const std::optional<block::Position>& highestClean) noexcept -> void;
    auto do_process_update(Message&& msg, allocator_type monotonic) noexcept
        -> void final;
    auto do_reorg(
        const node::HeaderOracle& oracle,
        const node::internal::HeaderOraclePrivate& data,
        Reorg::Params& params) noexcept -> bool final;
    auto do_startup_internal(allocator_type monotonic) noexcept -> void final;
    auto forward_to_next(Message&& msg) noexcept -> void final;
    auto process_clean(const Set<ScanStatus>& clean) noexcept -> void;
    auto process_dirty(const Set<block::Position>& dirty) noexcept -> void;
    auto process_do_rescan(Message&& in) noexcept -> void final;
    auto process_filter(
        Message&& in,
        block::Position&& tip,
        allocator_type monotonic) noexcept -> void final;
    auto prune() noexcept -> void;
    auto set_last_scanned(const block::Position& value) noexcept -> void;
    auto set_last_scanned(const std::optional<block::Position>& value) noexcept
        -> void;
    auto set_last_scanned(std::optional<block::Position>&& value) noexcept
        -> void;
    auto work(allocator_type monotonic) noexcept -> bool final;
};
}  // namespace opentxs::blockchain::node::wallet
