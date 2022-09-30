// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/ReorgSlave.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
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
namespace block
{
class Position;
}  // namespace block

namespace crypto
{
class Account;
class Deterministic;
class HD;
class Notification;
class PaymentCode;
}  // namespace crypto

namespace database
{
class Wallet;
}  // namespace database

namespace node
{
namespace internal
{
class Mempool;
struct HeaderOraclePrivate;
}  // namespace internal

namespace wallet
{
class Reorg;
class Subchain;
class SubchainStateData;
}  // namespace wallet

class HeaderOracle;
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
class Push;
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Account::Imp final : public opentxs::Actor<Imp, AccountJobs>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;

    Imp(Reorg& reorg,
        const crypto::Account& account,
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        std::string_view fromParent,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend opentxs::Actor<Imp, AccountJobs>;

    using SubchainsIDs = Set<identifier::Generic>;
    using HandledReorgs = Set<StateSequence>;
    using State = JobState;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    const api::Session& api_;
    const crypto::Account& account_;
    const node::Manager& node_;
    database::Wallet& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const cfilter::Type filter_type_;
    const CString from_parent_;
    State state_;
    HandledReorgs reorgs_;
    SubchainsIDs notification_;
    SubchainsIDs internal_;
    SubchainsIDs external_;
    SubchainsIDs outgoing_;
    SubchainsIDs incoming_;
    ReorgSlave reorg_;

    auto check(
        const crypto::Deterministic& subaccount,
        const crypto::Subchain subchain,
        SubchainsIDs& set) noexcept -> void;
    auto check_hd(const identifier::Generic& subaccount) noexcept -> void;
    auto check_hd(const crypto::HD& subaccount) noexcept -> void;
    auto check_notification(const identifier::Generic& subaccount) noexcept
        -> void;
    auto check_notification(const crypto::Notification& subaccount) noexcept
        -> void;
    auto check_pc(const identifier::Generic& subaccount) noexcept -> void;
    auto check_pc(const crypto::PaymentCode& subaccount) noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_reorg(
        const node::HeaderOracle& oracle,
        const node::internal::HeaderOraclePrivate& data,
        Reorg::Params& params) noexcept -> bool;
    auto do_startup() noexcept -> bool;
    auto index_nym(const identifier::Nym& id) noexcept -> void;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_key(Message&& in) noexcept -> void;
    auto process_prepare_reorg(Message&& in) noexcept -> void;
    auto process_rescan(Message&& in) noexcept -> void;
    auto process_subaccount(Message&& in) noexcept -> void;
    auto process_subaccount(
        const identifier::Generic& id,
        const crypto::SubaccountType type) noexcept -> void;
    auto scan_subchains() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_pre_shutdown(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> void;
    auto transition_state_pre_shutdown() noexcept -> void;
    auto transition_state_reorg(StateSequence id) noexcept -> void;
    auto work() noexcept -> bool;

    Imp(Reorg& reorg,
        const crypto::Account& account,
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        CString&& fromParent,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
