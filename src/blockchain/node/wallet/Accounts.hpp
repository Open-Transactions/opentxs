// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::node::wallet::Accounts::Imp
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/blockchain/node/wallet/ReorgMaster.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace database
{
class Wallet;
}  // namespace database

namespace node
{
namespace internal
{
class Mempool;
}  // namespace internal

namespace wallet
{
class Account;
class NotificationStateData;
class Subchain;
}  // namespace wallet

class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

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
using AccountsActor = opentxs::
    Actor<Accounts::Imp, AccountsJobs, OT_ZMQ_BLOCKCHAIN_WALLET_SHUTDOWN_READY>;

class Accounts::Imp final : public AccountsActor
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void
    {
        signal_startup(me);
    }

    Imp(std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend AccountsActor;

    enum class State {
        normal,
        pre_reorg,
        pre_shutdown,
        shutdown,
    };

    using Accounts = Set<identifier::Nym>;
    using Ancestor = block::Position;
    using Tip = block::Position;
    using Reorg = std::pair<Ancestor, Tip>;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    const api::Session& api_;
    const node::Manager& node_;
    database::Wallet& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const CString to_children_endpoint_;
    network::zeromq::socket::Raw& to_children_;
    State state_;
    StateSequence reorg_counter_;
    Accounts accounts_;
    std::optional<StateSequence> startup_reorg_;
    std::optional<Reorg> reorg_data_;
    ReorgMaster reorg_;

    auto do_reorg() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> bool;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_block_header(Message&& in) noexcept -> void;
    auto process_nym(Message&& in) noexcept -> void;
    auto process_nym(const identifier::Nym& nym) noexcept -> void;
    auto process_reorg(Message&& in) noexcept -> void;
    auto process_reorg(
        Message&& in,
        block::Position&& ancestor,
        block::Position&& tip) noexcept -> void;
    auto process_rescan(Message&& in) noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_pre_reorg(const Work work, Message&& msg) noexcept -> void;
    auto state_pre_shutdown(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> void;
    auto transition_state_pre_reorg() noexcept -> void;
    auto transition_state_pre_shutdown() noexcept -> void;
    auto work() noexcept -> bool;

    Imp(std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        network::zeromq::BatchID batch,
        CString&& toChildren,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
