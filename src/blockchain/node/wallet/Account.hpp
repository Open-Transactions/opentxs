// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/Subchain.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
class Deterministic;
class HD;
class PaymentCode;
}  // namespace crypto

namespace node
{
namespace internal
{
struct Network;
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Subchain;
class SubchainStateData;
}  // namespace wallet
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
class Push;
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Account::Imp final : public Actor<Imp, AccountJobs>
{
public:
    auto VerifyState(const State state) const noexcept -> void;

    auto ChangeState(const State state) noexcept -> bool;
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;
    auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> void;
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    Imp(const api::Session& api,
        const crypto::Account& account,
        const node::internal::Network& node,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const cfilter::Type filter,
        const std::string_view shutdown,
        allocator_type alloc) noexcept;

    ~Imp() final { signal_shutdown(); }

private:
    friend Actor<Imp, AccountJobs>;

    using Subchains = Map<OTIdentifier, boost::shared_ptr<SubchainStateData>>;
    using Subtype = node::internal::WalletDatabase::Subchain;

    const api::Session& api_;
    const crypto::Account& account_;
    const node::internal::Network& node_;
    node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const CString name_;
    const cfilter::Type filter_type_;
    const CString shutdown_endpoint_;
    std::atomic<State> state_;
    Subchains notification_;
    Subchains internal_;
    Subchains external_;
    Subchains outgoing_;
    Subchains incoming_;

    auto check_hd(const Identifier& subaccount) noexcept -> void;
    auto check_hd(const crypto::HD& subaccount) noexcept -> void;
    auto check_pc(const Identifier& subaccount) noexcept -> void;
    auto check_pc(const crypto::PaymentCode& subaccount) noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> void;
    template <typename Callback>
    auto for_each(const Callback& cb) noexcept -> void
    {
        std::for_each(notification_.begin(), notification_.end(), cb);
        std::for_each(internal_.begin(), internal_.end(), cb);
        std::for_each(external_.begin(), external_.end(), cb);
        std::for_each(outgoing_.begin(), outgoing_.end(), cb);
        std::for_each(incoming_.begin(), incoming_.end(), cb);
    }
    auto get(
        const crypto::Deterministic& subaccount,
        const Subtype subchain,
        Subchains& map) noexcept -> Subchain&;
    auto index_nym(const identifier::Nym& id) noexcept -> void;
    auto instantiate(
        const crypto::Deterministic& subaccount,
        const Subtype subchain,
        Subchains& map) noexcept -> Subchain&;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_key(Message&& in) noexcept -> void;
    auto process_subaccount(Message&& in) noexcept -> void;
    auto process_subaccount(
        const Identifier& id,
        const crypto::SubaccountType type) noexcept -> void;
    auto scan_subchains() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> void;
    auto transition_state_reorg() noexcept -> void;
    auto transition_state_shutdown() noexcept -> void;
    auto work() noexcept -> bool;

    Imp(const api::Session& api,
        const crypto::Account& account,
        const node::internal::Network& node,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const cfilter::Type filter,
        CString&& shutdown,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
