// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <optional>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Index::Imp : public statemachine::Job
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

    ~Imp() override = default;

protected:
    auto done(database::Wallet::ElementMap&& elements) noexcept -> void;

private:
    network::zeromq::socket::Raw& to_rescan_;
    network::zeromq::socket::Raw& to_scan_;
    std::optional<Bip32Index> last_indexed_;

    virtual auto need_index(const std::optional<Bip32Index>& current)
        const noexcept -> std::optional<Bip32Index> = 0;

    auto do_process_update(Message&& msg) noexcept -> void final;
    auto do_startup_internal() noexcept -> void final;
    auto forward_to_next(Message&& msg) noexcept -> void final;
    virtual auto process(
        const std::optional<Bip32Index>& current,
        Bip32Index target) noexcept -> void = 0;
    auto process_do_rescan(Message&& in) noexcept -> void final;
    auto process_filter(Message&& in, block::Position&& tip) noexcept
        -> void final;
    auto process_key(Message&& in) noexcept -> void final;
    auto work() noexcept -> bool final;
};
}  // namespace opentxs::blockchain::node::wallet
