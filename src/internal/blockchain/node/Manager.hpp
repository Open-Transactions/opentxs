// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "internal/blockchain/node/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/util/Container.hpp"

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
class Hash;
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
class FilterOracle;
class Mempool;
class PeerManager;
struct Config;
}  // namespace internal

struct Endpoints;
class FilterOracle;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Manager : virtual public node::Manager
{
public:
    virtual auto BroadcastTransaction(
        const block::Transaction& tx,
        const bool pushtx = false) const noexcept -> bool = 0;
    virtual auto Chain() const noexcept -> Type = 0;
    virtual auto DB() const noexcept -> database::Database& = 0;
    virtual auto Endpoints() const noexcept -> const node::Endpoints& = 0;
    // amount represents satoshis per 1000 bytes
    virtual auto FeeRate() const noexcept -> Amount = 0;
    virtual auto GetConfig() const noexcept -> const Config& = 0;
    // WARNING do not call until the Manager is fully constructed
    virtual auto GetShared() const noexcept
        -> std::shared_ptr<const node::Manager> = 0;
    virtual auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash> = 0;
    virtual auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash> = 0;
    virtual auto Mempool() const noexcept -> const internal::Mempool& = 0;
    virtual auto ShuttingDown() const noexcept -> bool = 0;

    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;
    virtual auto Start(std::shared_ptr<const node::Manager>) noexcept
        -> void = 0;
    virtual auto StartWallet() noexcept -> void = 0;

    ~Manager() override = default;
};
}  // namespace opentxs::blockchain::node::internal
