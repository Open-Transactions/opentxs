// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <robin_hood.h>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainAccountData.pb.h"
#include "serialization/protobuf/BlockchainActivity.pb.h"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Element;
}  // namespace crypto
}  // namespace blockchain

namespace crypto
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class AsymmetricKey;
class BlockchainAccountData;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::blockchain::crypto::implementation
{
class Subaccount : virtual public internal::Subaccount
{
public:
    auto AssociateTransaction(
        const std::pmr::vector<Activity>& unspent,
        const std::pmr::vector<Activity>& outgoing,
        std::pmr::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto ID() const noexcept -> const Identifier& final { return id_; }
    auto Internal() const noexcept -> internal::Subaccount& final
    {
        return const_cast<Subaccount&>(*this);
    }
    auto IncomingTransactions(const Key& key) const noexcept
        -> std::pmr::set<std::string> final;
    auto Parent() const noexcept -> const crypto::Account& final
    {
        return parent_;
    }
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept -> ECKey override
    {
        return {};
    }
    auto ScanProgress(Subchain type) const noexcept -> block::Position override;

    auto Confirm(
        const Subchain type,
        const Bip32Index index,
        const Txid& tx) noexcept -> bool final;
    auto SetContact(
        const Subchain type,
        const Bip32Index index,
        const Identifier& id) noexcept(false) -> bool final;
    auto SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string& label) noexcept(false) -> bool final;
    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void override
    {
    }
    auto Type() const noexcept -> SubaccountType final { return type_; }
    auto Unconfirm(
        const Subchain type,
        const Bip32Index index,
        const Txid& tx,
        const Time time) noexcept -> bool final;
    auto Unreserve(const Subchain type, const Bip32Index index) noexcept
        -> bool final;
    auto UpdateElement(std::pmr::vector<ReadView>& pubkeyHashes) const noexcept
        -> void final;

    ~Subaccount() override = default;

protected:
    using AddressMap =
        robin_hood::unordered_flat_map<Bip32Index, std::unique_ptr<Element>>;
    using Revision = std::uint64_t;

    struct AddressData {
        const Subchain type_;
        const bool set_contact_;
        block::Position progress_;
        AddressMap map_;

        AddressData(
            const api::Session& api,
            Subchain type,
            bool contact) noexcept;
    };

    const api::Session& api_;
    const Account& parent_;
    const opentxs::blockchain::Type chain_;
    const SubaccountType type_;
    const OTIdentifier id_;
    mutable std::recursive_mutex lock_;
    mutable std::atomic<Revision> revision_;
    mutable internal::ActivityMap unspent_;
    mutable internal::ActivityMap spent_;

    using SerializedActivity =
        google::protobuf::RepeatedPtrField<proto::BlockchainActivity>;
    using SerializedType = proto::BlockchainAccountData;

    static auto convert(Activity&& in) noexcept -> proto::BlockchainActivity;
    static auto convert(const proto::BlockchainActivity& in) noexcept
        -> Activity;
    static auto convert(const SerializedActivity& in) noexcept
        -> std::pmr::vector<Activity>;
    static auto convert(const std::pmr::vector<Activity>& in) noexcept
        -> internal::ActivityMap;

    virtual auto account_already_exists(const rLock& lock) const noexcept
        -> bool = 0;
    void process_spent(
        const rLock& lock,
        const Coin& coin,
        const Key key,
        const Amount value) const noexcept;
    void process_unspent(
        const rLock& lock,
        const Coin& coin,
        const Key key,
        const Amount value) const noexcept;
    virtual auto save(const rLock& lock) const noexcept -> bool = 0;
    auto serialize_common(const rLock& lock, SerializedType& out) const noexcept
        -> void;

    // NOTE call only from final constructor bodies
    auto init() noexcept -> void;
    virtual auto mutable_element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) -> Element& = 0;

    Subaccount(
        const api::Session& api,
        const Account& parent,
        const SubaccountType type,
        OTIdentifier&& id,
        Identifier& out) noexcept;
    Subaccount(
        const api::Session& api,
        const Account& parent,
        const SubaccountType type,
        const SerializedType& serialized,
        Identifier& out) noexcept(false);

private:
    static constexpr auto ActivityVersion = VersionNumber{1};
    static constexpr auto BlockchainAccountDataVersion = VersionNumber{1};

    virtual auto check_activity(
        const rLock& lock,
        const std::pmr::vector<Activity>& unspent,
        std::pmr::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool = 0;

    virtual auto confirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void
    {
    }
    virtual auto unconfirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void
    {
    }

    Subaccount(
        const api::Session& api,
        const Account& parent,
        const SubaccountType type,
        OTIdentifier&& id,
        const Revision revision,
        const std::pmr::vector<Activity>& unspent,
        const std::pmr::vector<Activity>& spent,
        Identifier& out) noexcept;
    Subaccount() = delete;
    Subaccount(const Subaccount&) = delete;
    Subaccount(Subaccount&&) = delete;
    auto operator=(const Subaccount&) -> Subaccount& = delete;
    auto operator=(Subaccount&&) -> Subaccount& = delete;
};
}  // namespace opentxs::blockchain::crypto::implementation
