// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_ordered_guarded.h>
#include <opentxs/protobuf/HDPath.pb.h>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <string_view>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/network/zeromq/Types.hpp"
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
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
}  // namespace block

namespace crypto
{
class Notification;
}  // namespace crypto

namespace node
{
namespace wallet
{
class Reorg;
}  // namespace wallet

class Manager;
}  // namespace node
}  // namespace blockchain

class Log;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class NotificationStateData final : public SubchainStateData
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    NotificationStateData(
        Reorg& reorg,
        crypto::Notification& subaccount,
        const opentxs::PaymentCode& code,
        std::shared_ptr<const api::internal::Session> api,
        std::shared_ptr<const node::Manager> node,
        crypto::Subchain subchain,
        std::string_view fromParent,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    NotificationStateData() = delete;
    NotificationStateData(const NotificationStateData&) = delete;
    NotificationStateData(NotificationStateData&&) = delete;
    auto operator=(const NotificationStateData&)
        -> NotificationStateData& = delete;
    auto operator=(NotificationStateData&&) -> NotificationStateData& = delete;

    ~NotificationStateData() final;

private:
    using PaymentCode =
        libguarded::ordered_guarded<opentxs::PaymentCode, std::shared_mutex>;
    using Cache = libguarded::ordered_guarded<Vector<block::Position>>;

    const protobuf::HDPath path_;
    const opentxs::PaymentCode pc_;
    const CString pc_display_;
    mutable PaymentCode pc_secret_;
    mutable Cache cache_;

    auto CheckCache(const std::size_t outstanding, FinishedCallback cb)
        const noexcept -> void final;
    auto do_startup(allocator_type monotonic) noexcept -> bool final;
    auto get_index(const std::shared_ptr<const SubchainStateData>& me)
        const noexcept -> void final;
    auto handle_block_matches(
        const block::Position& position,
        const block::Matches& confirmed,
        const Log& log,
        block::Block& block,
        allocator_type monotonic) const noexcept -> void final;
    auto handle_mempool_match(
        const block::Matches& matches,
        block::Transaction tx,
        allocator_type monotonic) const noexcept -> void final;
    auto init_keys(opentxs::PaymentCode& pc, const PasswordPrompt& reason)
        const noexcept -> void;
    auto process(
        const block::Match match,
        const block::Transaction& tx,
        bool confirmed,
        const PasswordPrompt& reason) const noexcept -> void;
    auto process(
        const block::TransactionHash& tx,
        const opentxs::PaymentCode& remote,
        bool confirmed,
        const PasswordPrompt& reason) const noexcept -> void;

    auto init_contacts(allocator_type monotonic) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool final;
};
}  // namespace opentxs::blockchain::node::wallet
