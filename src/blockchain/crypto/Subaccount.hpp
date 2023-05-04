// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"

#pragma once

#include <BlockchainAccountData.pb.h>
#include <BlockchainActivity.pb.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string_view>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

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
class TransactionHash;
}  // namespace block
}  // namespace blockchain
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Subaccount : virtual public internal::Subaccount
{
public:
    auto AssociateTransaction(
        const UnallocatedVector<Activity>& unspent,
        const UnallocatedVector<Activity>& outgoing,
        UnallocatedSet<identifier::Generic>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto Describe() const noexcept -> std::string_view final
    {
        return description_;
    }
    auto ID() const noexcept -> const identifier::Account& final { return id_; }
    auto Internal() const noexcept -> internal::Subaccount& final
    {
        return const_cast<Subaccount&>(*this);
    }
    auto IncomingTransactions(const Key& key) const noexcept
        -> UnallocatedSet<UnallocatedCString> final;
    auto Parent() const noexcept -> const crypto::Account& final
    {
        return parent_;
    }
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::crypto::asymmetric::key::EllipticCurve override;
    auto ScanProgress(Subchain type) const noexcept -> block::Position override;

    auto Confirm(
        const Subchain type,
        const Bip32Index index,
        const block::TransactionHash& tx) noexcept -> bool final;
    auto SetContact(
        const Subchain type,
        const Bip32Index index,
        const identifier::Generic& id) noexcept(false) -> bool final;
    auto SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string_view label) noexcept(false) -> bool final;
    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void override
    {
    }
    auto Type() const noexcept -> SubaccountType final { return type_; }
    auto Unconfirm(
        const Subchain type,
        const Bip32Index index,
        const block::TransactionHash& tx,
        const Time time) noexcept -> bool final;
    auto Unreserve(const Subchain type, const Bip32Index index) noexcept
        -> bool final;
    auto UpdateElement(UnallocatedVector<ReadView>& pubkeyHashes) const noexcept
        -> void final;

    Subaccount() = delete;
    Subaccount(const Subaccount&) = delete;
    Subaccount(Subaccount&&) = delete;
    auto operator=(const Subaccount&) -> Subaccount& = delete;
    auto operator=(Subaccount&&) -> Subaccount& = delete;

    ~Subaccount() override;

protected:
    using AddressMap = Map<Bip32Index, std::unique_ptr<Element>>;
    using Revision = std::uint64_t;

    struct AddressData {
        const Subchain type_;
        const bool set_contact_;
        block::Position progress_;
        AddressMap map_;

        auto check_keys() const noexcept -> bool;

        AddressData(
            const api::Session& api,
            Subchain type,
            bool contact) noexcept;
    };

    const api::Session& api_;
    const crypto::Account& parent_;
    const opentxs::blockchain::Type chain_;
    const SubaccountType type_;
    const identifier::Account id_;
    const CString description_;
    mutable std::recursive_mutex lock_;
    mutable std::atomic<Revision> revision_;
    mutable internal::ActivityMap unspent_;
    mutable internal::ActivityMap spent_;

    using SerializedActivity =
        google::protobuf::RepeatedPtrField<proto::BlockchainActivity>;
    using SerializedType = proto::BlockchainAccountData;

    static auto convert(const api::Session& api, Activity&& in) noexcept
        -> proto::BlockchainActivity;
    static auto convert(
        const api::Session& api,
        const proto::BlockchainActivity& in) noexcept -> Activity;
    static auto convert(
        const api::Session& api,
        const SerializedActivity& in) noexcept -> UnallocatedVector<Activity>;
    static auto convert(const UnallocatedVector<Activity>& in) noexcept
        -> internal::ActivityMap;
    static auto describe(
        const api::Session& api,
        const opentxs::blockchain::Type chain,
        const SubaccountType type,
        const identifier::Generic& id) noexcept -> CString;

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
    virtual auto init() noexcept -> void;
    virtual auto mutable_element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) -> Element& = 0;

    Subaccount(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        identifier::Account&& id,
        identifier::Account& out) noexcept;
    Subaccount(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        const SerializedType& serialized,
        identifier::Account& out) noexcept(false);

private:
    static constexpr auto ActivityVersion = VersionNumber{1};
    static constexpr auto BlockchainAccountDataVersion = VersionNumber{1};

    virtual auto check_activity(
        const rLock& lock,
        const UnallocatedVector<Activity>& unspent,
        UnallocatedSet<identifier::Generic>& contacts,
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
        const crypto::Account& parent,
        const SubaccountType type,
        identifier::Account&& id,
        const Revision revision,
        const UnallocatedVector<Activity>& unspent,
        const UnallocatedVector<Activity>& spent,
        identifier::Account& out) noexcept;
};
}  // namespace opentxs::blockchain::crypto::implementation
