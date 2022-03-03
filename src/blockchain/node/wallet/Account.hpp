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

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/BoostPMR.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "util/Actor.hpp"
#include "util/Gatekeeper.hpp"
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
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

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
class Account::Imp final : public opentxs::Actor<Imp, AccountJobs>
{
public:
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
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const filter::Type filter,
        const std::string_view shutdown,
        const std::string_view fromParent,
        const std::string_view toParent,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend opentxs::Actor<Imp, AccountJobs>;

    enum class State {
        normal,
        pre_reorg,
        reorg,
        post_reorg,
    };

    struct ReorgData {
        const std::size_t target_;
        std::size_t ready_;
        std::size_t done_;

        ReorgData(std::size_t target) noexcept
            : target_(target)
            , ready_(0)
            , done_(0)
        {
        }
    };
    using Subchains =
        Map<OTIdentifier, boost::shared_ptr<DeterministicStateData>>;
    using Subtype = node::internal::WalletDatabase::Subchain;

    const api::Session& api_;
    const crypto::Account& account_;
    const node::internal::Network& node_;
    const node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const filter::Type filter_type_;
    const CString shutdown_endpoint_;
    const CString to_children_endpoint_;
    const CString from_children_endpoint_;
    network::zeromq::socket::Raw& to_parent_;
    network::zeromq::socket::Raw& to_children_;
    State state_;
    std::optional<ReorgData> reorg_;
    Subchains internal_;
    Subchains external_;
    Subchains outgoing_;
    Subchains incoming_;

    auto reorg_children() const noexcept -> std::size_t;

    auto check_hd(const Identifier& subaccount) noexcept -> void;
    auto check_hd(const crypto::HD& subaccount) noexcept -> void;
    auto check_pc(const Identifier& subaccount) noexcept -> void;
    auto check_pc(const crypto::PaymentCode& subaccount) noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto get(
        const crypto::Deterministic& subaccount,
        const Subtype subchain,
        Subchains& map) noexcept -> Subchain&;
    auto instantiate(
        const crypto::Deterministic& subaccount,
        const Subtype subchain,
        Subchains& map) noexcept -> Subchain&;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_key(Message&& in) noexcept -> void;
    auto ready_for_normal() noexcept -> void;
    auto ready_for_reorg() noexcept -> void;
    auto scan_subchains() noexcept -> void;
    auto startup() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_post_reorg(const Work work, Message&& msg) noexcept -> void;
    auto state_pre_reorg(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal(Message&& in) noexcept -> void;
    auto transition_state_post_reorg(Message&& in) noexcept -> void;
    auto transition_state_pre_reorg(Message&& in) noexcept -> void;
    auto transition_state_reorg(Message&& in) noexcept -> void;
    auto work() noexcept -> bool;

    Imp(const api::Session& api,
        const crypto::Account& account,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const filter::Type filter,
        const std::string_view shutdown,
        const std::string_view fromParent,
        const std::string_view toParent,
        CString&& fromChildren,
        CString&& toChildren,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
