// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainEthereumAccountData.pb.h>
#include <HDPath.pb.h>
#include <boost/container/flat_set.hpp>
#include <memory>
#include <optional>

#include "blockchain/crypto/subaccount/imported/Imp.hpp"
#include "internal/blockchain/crypto/Ethereum.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Ethereum.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/ethereum/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
namespace internal
{
class Subaccount;
}  // namespace internal

class Account;
}  // namespace crypto
}  // namespace blockchain

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class EthereumPrivate final : public internal::Ethereum, public ImportedPrivate
{
public:
    using SerializedType = proto::BlockchainEthereumAccountData;

    auto asImportedPublic() const noexcept -> const crypto::Imported& final
    {
        return const_cast<EthereumPrivate*>(this)->asEthereumPublic();
    }
    auto asEthereum() const noexcept -> const internal::Ethereum& final
    {
        return *this;
    }
    auto asEthereumPublic() const noexcept -> const crypto::Ethereum& final
    {
        return const_cast<EthereumPrivate*>(this)->asEthereumPublic();
    }
    auto Balance() const noexcept -> Amount final;
    auto KnownIncoming(alloc::Strategy alloc) const noexcept
        -> Set<block::TransactionHash> final;
    auto KnownOutgoing(alloc::Strategy alloc) const noexcept
        -> Set<block::TransactionHash> final;
    auto MissingOutgoing(alloc::Strategy alloc) const noexcept
        -> Set<protocol::ethereum::AccountNonce> final;
    auto NextOutgoing() const noexcept
        -> protocol::ethereum::AccountNonce final;
    auto Self() const noexcept -> const crypto::Subaccount& final
    {
        return asEthereumPublic();
    }

    auto AddIncoming(
        const Amount& balance,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool final;
    auto AddIncoming(
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool final;
    auto AddOutgoing(
        const Amount& balance,
        protocol::ethereum::AccountNonce nonce,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool final;
    auto AddOutgoing(
        protocol::ethereum::AccountNonce nonce,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool final;
    auto asImportedPublic() noexcept -> crypto::Imported& final
    {
        return asEthereumPublic();
    }
    auto asEthereum() noexcept -> internal::Ethereum& final { return *this; }
    auto asEthereumPublic() noexcept -> crypto::Ethereum& final
    {
        return *self_;
    }
    auto InitSelf(std::shared_ptr<Subaccount> me) noexcept -> void final;
    auto Self() noexcept -> crypto::Subaccount& final
    {
        return asEthereumPublic();
    }
    auto UpdateBalance(const Amount& balance) noexcept -> bool final;

    EthereumPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const proto::HDPath& path,
        const HDProtocol standard,
        const PasswordPrompt& reason,
        opentxs::crypto::SeedID seed) noexcept(false);
    EthereumPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const identifier::Account& id,
        const SerializedType& proto,
        opentxs::crypto::SeedID seed) noexcept(false);
    EthereumPrivate(const EthereumPrivate&) = delete;
    EthereumPrivate(EthereumPrivate&&) = delete;
    auto operator=(const EthereumPrivate&) -> EthereumPrivate& = delete;
    auto operator=(EthereumPrivate&&) -> EthereumPrivate& = delete;

    ~EthereumPrivate() final;

private:
    using Me = std::optional<crypto::Ethereum>;

    static constexpr auto default_version_ = VersionNumber{1};

    const proto::HDPath path_;
    const HDProtocol standard_;
    VersionNumber version_;
    Amount balance_;
    protocol::ethereum::AccountNonce next_;
    boost::container::flat_set<protocol::ethereum::AccountNonce> known_;
    boost::container::flat_set<block::TransactionHash> incoming_;
    boost::container::flat_set<block::TransactionHash> outgoing_;
    mutable Me self_;

    auto account_already_exists(const rLock& lock) const noexcept -> bool final;
    auto save(const rLock& lock) const noexcept -> bool final;

    auto add_incoming(
        const rLock& lock,
        const Amount& balance,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto add_incoming(
        const rLock& lock,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto add_outgoing(
        const rLock& lock,
        const Amount& balance,
        protocol::ethereum::AccountNonce nonce,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto add_outgoing(
        const rLock& lock,
        protocol::ethereum::AccountNonce nonce,
        const block::TransactionHash& txid,
        bool confirmed) noexcept -> bool;
    auto update_balance(const rLock& lock, const Amount& balance) noexcept
        -> bool;
};
}  // namespace opentxs::blockchain::crypto
