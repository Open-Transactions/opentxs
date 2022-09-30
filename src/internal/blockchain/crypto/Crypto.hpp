// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#pragma once

#include <BlockchainAddress.pb.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Crypto;
class Factory;
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace crypto
{
class Account;
class Deterministic;
class HD;
class Imported;
class Notification;
class PaymentCode;
class Subaccount;
class Wallet;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class HDPath;
}  // namespace proto

class ByteArray;
class Data;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto blockchain_thread_item_id(
    const api::Crypto& crypto,
    const api::Factory& factory,
    const opentxs::blockchain::Type chain,
    const Data& txid) noexcept -> identifier::Generic;
}  // namespace opentxs

namespace opentxs::blockchain::crypto
{
using Chain = opentxs::blockchain::Type;
}  // namespace opentxs::blockchain::crypto

namespace opentxs::blockchain::crypto::internal
{
using ActivityMap = UnallocatedMap<Coin, std::pair<Key, Amount>>;

struct Wallet : virtual public crypto::Wallet {
    virtual auto AddHDNode(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        identifier::Generic& id) noexcept -> bool = 0;

    ~Wallet() override = default;
};

struct Account : virtual public crypto::Account {
    virtual auto AssociateTransaction(
        const UnallocatedVector<Activity>& unspent,
        const UnallocatedVector<Activity>& spent,
        UnallocatedSet<identifier::Generic>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto ClaimAccountID(
        const identifier::Generic& id,
        crypto::Subaccount* node) const noexcept -> void = 0;
    virtual auto FindNym(const identifier::Nym& id) const noexcept -> void = 0;
    virtual auto LookupUTXO(const Coin& coin) const noexcept
        -> std::optional<std::pair<Key, Amount>> = 0;

    virtual auto AddHDNode(
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        identifier::Generic& id) noexcept -> bool = 0;
    virtual auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason,
        identifier::Generic& id) noexcept -> bool = 0;
    virtual auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::Txid& notification,
        const PasswordPrompt& reason,
        identifier::Generic& id) noexcept -> bool = 0;
    virtual auto Startup() noexcept -> void = 0;

    ~Account() override = default;
};

struct Element : virtual public crypto::Element {
    using Txid = opentxs::blockchain::block::Txid;
    using SerializedType = proto::BlockchainAddress;

    enum class Availability {
        NeverUsed,
        Reissue,
        StaleUnconfirmed,
        MetadataConflict,
        Reserved,
        Used,
    };

    virtual auto Elements() const noexcept -> UnallocatedSet<ByteArray> = 0;
    virtual auto ID() const noexcept -> const identifier::Generic& = 0;
    virtual auto IncomingTransactions() const noexcept
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto IsAvailable(
        const identifier::Generic& contact,
        const std::string_view memo) const noexcept -> Availability = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Serialize() const noexcept -> SerializedType = 0;

    virtual auto Confirm(const Txid& tx) noexcept -> bool = 0;
    virtual auto Reserve(const Time time) noexcept -> bool = 0;
    virtual auto SetContact(const identifier::Generic& id) noexcept -> void = 0;
    virtual auto SetLabel(const std::string_view label) noexcept -> void = 0;
    virtual auto SetMetadata(
        const identifier::Generic& contact,
        const std::string_view label) noexcept -> void = 0;
    virtual auto Unconfirm(const Txid& tx, const Time time) noexcept
        -> bool = 0;
    virtual auto Unreserve() noexcept -> bool = 0;
};

struct Subaccount : virtual public crypto::Subaccount {
    virtual auto AssociateTransaction(
        const UnallocatedVector<Activity>& unspent,
        const UnallocatedVector<Activity>& spent,
        UnallocatedSet<identifier::Generic>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto IncomingTransactions(const Key& key) const noexcept
        -> UnallocatedSet<UnallocatedCString> = 0;
    virtual auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept -> ECKey = 0;

    virtual auto Confirm(
        const Subchain type,
        const Bip32Index index,
        const Txid& tx) noexcept -> bool = 0;
    virtual auto SetContact(
        const Subchain type,
        const Bip32Index index,
        const identifier::Generic& id) noexcept(false) -> bool = 0;
    virtual auto SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string_view label) noexcept(false) -> bool = 0;
    virtual auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void = 0;
    virtual auto UpdateElement(
        UnallocatedVector<ReadView>& pubkeyHashes) const noexcept -> void = 0;
    virtual auto Unconfirm(
        const Subchain type,
        const Bip32Index index,
        const Txid& tx,
        const Time time) noexcept -> bool = 0;
    virtual auto Unreserve(const Subchain type, const Bip32Index index) noexcept
        -> bool = 0;
};

struct Deterministic : virtual public crypto::Deterministic,
                       virtual public Subaccount {
};

struct HD : virtual public crypto::HD, virtual public Deterministic {
};

struct Imported : virtual public crypto::Imported, virtual public Subaccount {
};

struct Notification : virtual public crypto::Notification,
                      virtual public Subaccount {
    auto InternalNotification() const noexcept
        -> const internal::Notification& final
    {
        return *this;
    }
    virtual auto Path() const noexcept -> proto::HDPath = 0;

    auto InternalNotification() noexcept -> internal::Notification& final
    {
        return *this;
    }
};

struct PaymentCode : virtual public crypto::PaymentCode,
                     virtual public Deterministic {
    static auto GetID(
        const api::Session& api,
        const Chain chain,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote) noexcept -> identifier::Generic;
};
}  // namespace opentxs::blockchain::crypto::internal
