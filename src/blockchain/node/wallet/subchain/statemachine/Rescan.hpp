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
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace node
{
namespace wallet
{
class SubchainStateData;
}  // namespace wallet
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Rescan::Imp final : public statemachine::Job
{
public:
    auto ProcessReorg(const block::Position& parent) noexcept -> void final;

    Imp(const SubchainStateData& parent,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;

    ~Imp() final = default;

private:
    network::zeromq::socket::Raw& to_process_;
    network::zeromq::socket::Raw& to_progress_;
    bool active_;
    std::optional<block::Position> last_scanned_;
    std::optional<block::Position> filter_tip_;
    Set<block::Position> dirty_;

    auto caught_up() const noexcept -> bool;
    auto highest_clean(const Set<block::Position>& clean) const noexcept
        -> std::optional<block::Position>;
    auto stop() const noexcept -> block::Height;

    auto adjust_last_scanned(
        std::optional<block::Position>&& highestClean) noexcept -> void;
    auto do_startup() noexcept -> void final;
    auto process(const Set<ScanStatus>& clean) noexcept -> void;
    auto process_filter(block::Position&& tip) noexcept -> void final;
    auto process_update(Message&& msg) noexcept -> void final;
    auto prune() noexcept -> void;
    auto work() noexcept -> bool final;
};
}  // namespace opentxs::blockchain::node::wallet
