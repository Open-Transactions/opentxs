// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <future>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class PaymentCode;
}  // namespace crypto

namespace node
{
namespace manager
{
class Actor;
class Shared;
}  // namespace manager

class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
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

class Frame;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionProposal;
class HDPath;
}  // namespace proto

class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::manager
{
using namespace std::literals;
using ManagerActor = opentxs::Actor<manager::Actor, ManagerJobs>;

class Actor final : public ManagerActor
{
public:
    auto Init(boost::shared_ptr<Actor> me) noexcept -> std::future<void>;

    Actor(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<node::Manager> self,
        std::shared_ptr<Shared> shared,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend ManagerActor;

    static constexpr auto proposal_version_ = VersionNumber{1};
    static constexpr auto notification_version_ = VersionNumber{1};
    static constexpr auto output_version_ = VersionNumber{1};
    static constexpr auto sweep_version_ = VersionNumber{1};
    static constexpr auto sweep_key_version_ = VersionNumber{1};
    static constexpr auto heartbeat_interval_ = 5s;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<node::Manager> self_p_;
    std::shared_ptr<Shared> shared_p_;
    const api::Session& api_;
    node::Manager& self_;
    Shared& shared_;
    network::zeromq::socket::Raw& to_peer_manager_;
    network::zeromq::socket::Raw& to_wallet_;
    network::zeromq::socket::Raw& to_dht_;
    network::zeromq::socket::Raw& to_blockchain_api_;
    Timer heartbeat_;
    std::promise<void> init_;

    static auto serialize_notification(
        const PaymentCode& sender,
        const PaymentCode& recipient,
        const proto::HDPath& senderPath,
        proto::BlockchainTransactionProposal& out) noexcept -> void;

    auto create_or_load_subaccount(
        const identifier::Nym& senderNym,
        const PaymentCode& senderPC,
        const proto::HDPath& senderPath,
        const PaymentCode& recipient,
        const PasswordPrompt& reason,
        Set<PaymentCode>& notify) const noexcept -> const crypto::PaymentCode&;
    auto extract_notifications(
        const std::span<const network::zeromq::Frame> message,
        const identifier::Nym& senderNym,
        const PaymentCode& senderPC,
        const proto::HDPath& senderPath,
        const PasswordPrompt& reason,
        SendResult& rc,
        alloc::Strategy alloc) const noexcept(false) -> Set<PaymentCode>;
    auto get_sender(const identifier::Nym& nymID, SendResult& rc) const
        noexcept(false) -> std::pair<opentxs::PaymentCode, proto::HDPath>;
    auto notify_sync_client() const noexcept -> void;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_filter_update(Message&& in, allocator_type) noexcept -> void;
    auto process_heartbeat(Message&& in, allocator_type) noexcept -> void;
    auto process_send_to_address(Message&& in, allocator_type) noexcept -> void;
    auto process_send_to_payment_code(Message&& in, allocator_type) noexcept
        -> void;
    auto process_start_wallet(Message&& in, allocator_type) noexcept -> void;
    auto process_sweep(Message&& in, allocator_type) noexcept -> void;
    auto process_sync_data(Message&& in, allocator_type) noexcept -> void;
    auto reset_heartbeat() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::manager
