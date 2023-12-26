// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <memory>
#include <mutex>
#include <string_view>

#include "blockchain/node/manager/Data.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Shutdown.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
class Transaction;
class TransactionHash;
}  // namespace block

namespace database
{
class Database;
}  // namespace database

namespace node
{
namespace internal
{
class Mempool;
struct Config;
}  // namespace internal

class BlockOracle;
class FilterOracle;
class HeaderOracle;
class Manager;
class Mempool;
class Wallet;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain
}  // namespace network

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::manager
{
class Shared
{
public:
    auto AddBlock(const block::Block& block) const noexcept -> bool;
    auto AddPeer(const network::blockchain::Address& address) const noexcept
        -> bool;
    auto BlockOracle() const noexcept -> const node::BlockOracle&;
    auto BroadcastTransaction(const block::Transaction& tx, const bool pushtx)
        const noexcept -> bool;
    auto Chain() const noexcept -> Type;
    auto DB() const noexcept -> database::Database&;
    auto Endpoints() const noexcept -> const node::Endpoints&;
    auto FeeRate() const noexcept -> Amount;
    auto FilterOracle() const noexcept -> const node::FilterOracle&;
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetConfig() const noexcept -> const internal::Config&;
    auto GetShared() const noexcept -> std::shared_ptr<const node::Manager>;
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto HeaderOracle() const noexcept -> const node::HeaderOracle&;
    auto Listen(const network::blockchain::Address& address) const noexcept
        -> bool;
    auto Mempool() const noexcept -> const internal::Mempool&;
    auto Profile() const noexcept -> BlockchainProfile;
    auto ShuttingDown() const noexcept -> bool;

    auto FilterOracle() noexcept -> node::FilterOracle&;
    auto HeaderOracle() noexcept -> node::HeaderOracle&;
    auto Init(std::shared_ptr<node::Manager> self) noexcept -> void;
    auto Mempool() noexcept -> internal::Mempool&;
    auto Shutdown() noexcept -> void;
    auto StartWallet() noexcept -> void;
    auto Wallet() const noexcept -> const node::Wallet&;

    Shared(
        const api::session::Client& api,
        const Type type,
        const node::internal::Config& config,
        std::string_view seednode,
        node::Endpoints endpoints) noexcept;

    ~Shared();

private:
    using GuardedData = libguarded::plain_guarded<Data>;

    const api::session::Client& api_;
    const Type chain_;
    const node::internal::Config& config_;
    const node::Endpoints endpoints_;
    const cfilter::Type filter_type_;
    const CString command_line_peers_;
    opentxs::internal::ShutdownSender shutdown_sender_;
    std::shared_ptr<blockchain::database::Database> database_;
    std::unique_ptr<node::Mempool> mempool_;
    mutable node::internal::HeaderOracle header_;
    node::internal::BlockOracle block_;
    std::unique_ptr<node::FilterOracle> filter_;
    node::internal::Wallet wallet_;
    std::once_flag shutdown_;
    mutable GuardedData data_;
};
}  // namespace opentxs::blockchain::node::manager
