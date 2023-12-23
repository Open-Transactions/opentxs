// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <HDPath.pb.h>
#include <memory>
#include <string_view>

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/ReorgSlave.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Client;
}  // namespace internal

class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
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

class HeaderOracle;
class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Contact;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Account::Imp final : public opentxs::Actor<Imp, AccountJobs>
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Init(std::shared_ptr<Imp> me) noexcept -> void;

    Imp(Reorg& reorg,
        const crypto::Account& account,
        std::shared_ptr<const api::session::internal::Client> api,
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

    std::shared_ptr<const api::session::internal::Client> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    const api::session::Client& api_;
    const crypto::Account& account_;
    const node::Manager& node_;
    database::Wallet& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const cfilter::Type filter_type_;
    const CString from_parent_;
    const Nym_p nym_;
    const opentxs::PaymentCode local_;
    const proto::HDPath path_;
    const identifier::Generic self_contact_;
    State state_;
    HandledReorgs reorgs_;
    SubchainsIDs notification_;
    SubchainsIDs internal_;
    SubchainsIDs external_;
    SubchainsIDs outgoing_;
    SubchainsIDs incoming_;
    ReorgSlave reorg_;

    auto check(
        crypto::Deterministic& subaccount,
        const crypto::Subchain subchain,
        SubchainsIDs& set) noexcept -> void;
    auto check_hd(const identifier::Account& subaccount) noexcept -> void;
    auto check_hd(crypto::HD& subaccount) noexcept -> void;
    auto check_notification(const identifier::Account& subaccount) noexcept
        -> void;
    auto check_notification(crypto::Notification& subaccount) noexcept -> void;
    auto check_pc(const identifier::Account& subaccount) noexcept -> void;
    auto check_pc(crypto::PaymentCode& subaccount) noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_reorg(
        const node::HeaderOracle& oracle,
        const node::internal::HeaderOraclePrivate& data,
        Reorg::Params& params) noexcept -> bool;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto index_nym(const identifier::Nym& id) noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_contact(Message&& in, allocator_type monotonic) noexcept
        -> void;
    auto process_contact(
        const identifier::Generic& id,
        allocator_type monotonic) noexcept -> void;
    auto process_contact(
        const opentxs::Contact& contact,
        allocator_type monotonic) noexcept -> void;
    auto process_key(Message&& in) noexcept -> void;
    auto process_prepare_reorg(Message&& in) noexcept -> void;
    auto process_rescan(Message&& in) noexcept -> void;
    auto process_subaccount(Message&& in) noexcept -> void;
    auto process_subaccount(
        const identifier::Account& id,
        const crypto::SubaccountType type) noexcept -> void;
    auto scan_contacts(allocator_type monotonic) noexcept -> void;
    auto scan_subchains() noexcept -> void;
    auto state_normal(
        const Work work,
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto state_pre_shutdown(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> void;
    auto transition_state_pre_shutdown() noexcept -> void;
    auto transition_state_reorg(StateSequence id) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;

    Imp(Reorg& reorg,
        const crypto::Account& account,
        std::shared_ptr<const api::session::internal::Client> api,
        std::shared_ptr<const node::Manager> node,
        CString&& fromParent,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
