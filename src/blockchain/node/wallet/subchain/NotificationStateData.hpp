// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <HDPath.pb.h>
#include <cs_ordered_guarded.h>
#include <cs_shared_guarded.h>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <string_view>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
template <class T>
class shared_ptr;
}  // namespace boost

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
    NotificationStateData(
        Reorg& reorg,
        const crypto::Notification& subaccount,
        const opentxs::PaymentCode& code,
        std::shared_ptr<const api::Session> api,
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
        libguarded::shared_guarded<opentxs::PaymentCode, std::shared_mutex>;
    using Cache =
        libguarded::ordered_guarded<Vector<block::Position>, std::shared_mutex>;

    const proto::HDPath path_;
    const CString pc_display_;
    mutable PaymentCode code_;
    mutable Cache cache_;

    auto CheckCache(const std::size_t outstanding, FinishedCallback cb)
        const noexcept -> void final;
    auto do_startup(allocator_type monotonic) noexcept -> bool final;
    auto get_index(const boost::shared_ptr<const SubchainStateData>& me)
        const noexcept -> void final;
    auto handle_confirmed_matches(
        const block::Block& block,
        const block::Position& position,
        const block::Matches& confirmed,
        const Log& log,
        allocator_type monotonic) const noexcept -> void final;
    auto handle_mempool_matches(
        const block::Matches& matches,
        block::Transaction tx,
        allocator_type monotonic) const noexcept -> void final;
    auto init_keys() const noexcept -> PasswordPrompt;
    auto process(
        const block::Match match,
        const block::Transaction& tx,
        const PasswordPrompt& reason) const noexcept -> void;
    auto process(
        const opentxs::PaymentCode& remote,
        const PasswordPrompt& reason) const noexcept -> void;

    auto init_contacts(allocator_type monotonic) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool final;
};
}  // namespace opentxs::blockchain::node::wallet
