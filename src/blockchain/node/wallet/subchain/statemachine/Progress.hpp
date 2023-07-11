// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/wallet/subchain/statemachine/Progress.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/network/zeromq/Types.hpp"

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
class Progress::Imp final : public statemachine::Job
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }

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
    network::zeromq::socket::Raw& to_scan_;

    auto notify(const block::Position& pos) const noexcept -> void;

    auto do_process_update(Message&& msg, allocator_type monotonic) noexcept
        -> void final;
    auto do_reorg(
        const node::HeaderOracle& oracle,
        const node::internal::HeaderOraclePrivate& data,
        Reorg::Params& params) noexcept -> bool final;
    auto do_startup_internal(allocator_type monotonic) noexcept -> void final {}
    auto forward_to_next(Message&& msg) noexcept -> void final {}
    auto process_do_rescan(Message&& in) noexcept -> void final;
};
}  // namespace opentxs::blockchain::node::wallet
