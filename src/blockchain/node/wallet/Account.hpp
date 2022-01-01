// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <memory>

#include "blockchain/node/wallet/Actor.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "util/LMDB.hpp"

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
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace crypto
{
class Account;
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
class Accounts;
class DeterministicStateData;
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
class Push;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Identifier;
class Outstanding;
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class Account final : public Actor
{
public:
    using BalanceTree = crypto::Account;

    auto FinishBackgroundTasks() noexcept -> void final;
    auto ProcessBlockAvailable(const block::Hash& block) noexcept -> void final;
    auto ProcessKey() noexcept -> void final;
    auto ProcessMempool(
        std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void final;
    auto ProcessNewFilter(const block::Position& tip) noexcept -> void final;
    auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> bool final;
    auto ProcessStateMachine(bool enabled) noexcept -> bool final;
    auto Shutdown() noexcept -> void final;
    auto ProcessTaskComplete(
        const Identifier& id,
        const char* type,
        bool enabled) noexcept -> void final;

    Account(
        const api::Session& api,
        const BalanceTree& ref,
        const node::internal::Network& node,
        Accounts& parent,
        const node::internal::WalletDatabase& db,
        const filter::Type filter,
        Outstanding&& jobs,
        const std::function<void(const Identifier&, const char*)>&
            taskFinished) noexcept;
    Account(Account&&) noexcept;

    ~Account() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Account(const Account&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
