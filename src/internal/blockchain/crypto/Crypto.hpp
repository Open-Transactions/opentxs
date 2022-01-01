// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainAddress.pb.h"

namespace std
{
using COIN = std::tuple<std::string, std::size_t, std::int64_t>;

template <>
struct less<COIN> {
    auto operator()(const COIN& lhs, const COIN& rhs) const -> bool
    {
        const auto& [lID, lIndex, lAmount] = lhs;
        const auto& [rID, rIndex, rAmount] = rhs;

        if (lID < rID) { return true; }

        if (rID < lID) { return false; }

        if (lIndex < rIndex) { return true; }

        if (rIndex < lIndex) { return false; }

        return (lAmount < rAmount);
    }
};
}  // namespace std

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Crypto;
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
class Deterministic;
class HD;
class Imported;
class PaymentCode;
class Subaccount;
class Wallet;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class HDPath;
}  // namespace proto

class Identifier;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
auto blockchain_thread_item_id(
    const api::Crypto& crypto,
    const opentxs::blockchain::Type chain,
    const Data& txid) noexcept -> OTIdentifier;
}  // namespace opentxs

namespace opentxs::blockchain::crypto
{
using Chain = opentxs::blockchain::Type;
}  // namespace opentxs::blockchain::crypto

namespace opentxs::blockchain::crypto::internal
{
using ActivityMap = std::pmr::map<Coin, std::pair<Key, Amount>>;

struct Wallet : virtual public crypto::Wallet {
    virtual auto AddHDNode(
        const identifier::Nym& nym,
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        Identifier& id) noexcept -> bool = 0;

    ~Wallet() override = default;
};

struct Account : virtual public crypto::Account {
    virtual auto AssociateTransaction(
        const std::pmr::vector<Activity>& unspent,
        const std::pmr::vector<Activity>& spent,
        std::pmr::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto ClaimAccountID(const std::string& id, crypto::Subaccount* node)
        const noexcept -> void = 0;
    virtual auto FindNym(const identifier::Nym& id) const noexcept -> void = 0;
    virtual auto LookupUTXO(const Coin& coin) const noexcept
        -> std::optional<std::pair<Key, Amount>> = 0;

    virtual auto AddHDNode(
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        Identifier& id) noexcept -> bool = 0;
    virtual auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason,
        Identifier& id) noexcept -> bool = 0;
    virtual auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::Txid& notification,
        const PasswordPrompt& reason,
        Identifier& id) noexcept -> bool = 0;

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

    virtual auto Elements() const noexcept -> std::pmr::set<OTData> = 0;
    virtual auto ID() const noexcept -> const Identifier& = 0;
    virtual auto IncomingTransactions() const noexcept
        -> std::pmr::set<std::string> = 0;
    virtual auto IsAvailable(const Identifier& contact, const std::string& memo)
        const noexcept -> Availability = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Serialize() const noexcept -> SerializedType = 0;

    virtual auto Confirm(const Txid& tx) noexcept -> bool = 0;
    virtual auto Reserve(const Time time) noexcept -> bool = 0;
    virtual auto SetContact(const Identifier& id) noexcept -> void = 0;
    virtual auto SetLabel(const std::string& label) noexcept -> void = 0;
    virtual auto SetMetadata(
        const Identifier& contact,
        const std::string& label) noexcept -> void = 0;
    virtual auto Unconfirm(const Txid& tx, const Time time) noexcept
        -> bool = 0;
    virtual auto Unreserve() noexcept -> bool = 0;
};

struct Subaccount : virtual public crypto::Subaccount {
    virtual auto AssociateTransaction(
        const std::pmr::vector<Activity>& unspent,
        const std::pmr::vector<Activity>& spent,
        std::pmr::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto IncomingTransactions(const Key& key) const noexcept
        -> std::pmr::set<std::string> = 0;
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
        const Identifier& id) noexcept(false) -> bool = 0;
    virtual auto SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string& label) noexcept(false) -> bool = 0;
    virtual auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void = 0;
    virtual auto UpdateElement(
        std::pmr::vector<ReadView>& pubkeyHashes) const noexcept -> void = 0;
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

struct PaymentCode : virtual public crypto::PaymentCode,
                     virtual public Deterministic {
    static auto GetID(
        const api::Session& api,
        const Chain chain,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote) noexcept -> OTIdentifier;
};
}  // namespace opentxs::blockchain::crypto::internal
