// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal

class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace internal
{
struct WalletDatabase;
}  // namespace internal
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
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::blockchain::node
{
class Mempool final : public internal::Mempool
{
public:
    auto Dump() const noexcept -> std::pmr::set<std::string> final;
    auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const block::bitcoin::Transaction> final;
    auto Submit(ReadView txid) const noexcept -> bool final;
    auto Submit(const std::pmr::vector<ReadView>& txids) const noexcept
        -> std::pmr::vector<bool> final;
    auto Submit(std::unique_ptr<const block::bitcoin::Transaction> tx)
        const noexcept -> void final;

    auto Heartbeat() noexcept -> void final;

    Mempool(
        const api::crypto::Blockchain& crypto,
        const internal::WalletDatabase& wallet,
        const network::zeromq::socket::Publish& socket,
        const Type chain) noexcept;

    ~Mempool() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Mempool() = delete;
    Mempool(const Mempool&) = delete;
    Mempool(Mempool&&) = delete;
    auto operator=(const Mempool&) -> Mempool& = delete;
    auto operator=(Mempool&&) -> Mempool& = delete;
};
}  // namespace opentxs::blockchain::node
