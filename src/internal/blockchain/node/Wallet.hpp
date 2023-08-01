// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <future>
#include <memory>
#include <optional>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
class Outpoint;
}  // namespace block

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class Wallet final : virtual public node::Wallet
{
public:
    class Actor;
    class Shared;

    auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void;
    auto FeeEstimate() const noexcept -> std::optional<Amount>;
    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetBalance(
        const identifier::Nym& owner,
        const identifier::Account& subaccount) const noexcept -> Balance final;
    auto GetOutputs(TxoState type, alloc::Default alloc) const noexcept
        -> Vector<UTXO> final;
    auto GetOutputs(alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetOutputs(const crypto::Key& key, TxoState type, alloc::Default alloc)
        const noexcept -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetOutputs(const identifier::Nym& owner, alloc::Default alloc)
        const noexcept -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Account& subaccount,
        TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Account& subaccount,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final;
    auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag> final;
    auto Height() const noexcept -> block::Height final;
    auto Internal() const noexcept -> const internal::Wallet& final
    {
        return *this;
    }
    auto StartRescan() const noexcept -> bool final;

    auto Init(
        std::shared_ptr<const api::session::Client> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void;
    auto Internal() noexcept -> internal::Wallet& final { return *this; }

    Wallet() noexcept;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;

    ~Wallet() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::blockchain::node::internal
