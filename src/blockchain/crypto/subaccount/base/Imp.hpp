// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"

#pragma once

#include <BlockchainAccountData.pb.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string_view>

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

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

namespace proto
{
class BlockchainActivity;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class SubaccountPrivate : virtual public internal::Subaccount
{
public:
    auto Describe() const noexcept -> std::string_view final
    {
        return description_;
    }
    auto DisplayName() const noexcept -> std::string_view final
    {
        return display_name_;
    }
    auto ID() const noexcept -> const identifier::Account& final { return id_; }
    auto IsValid() const noexcept -> bool final { return true; }
    auto Parent() const noexcept -> const crypto::Account& final
    {
        return parent_;
    }
    auto Source() const noexcept -> const identifier::Generic& final
    {
        return source_;
    }
    auto SourceDescription() const noexcept -> std::string_view final
    {
        return source_description_;
    }

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

    SubaccountPrivate() = delete;
    SubaccountPrivate(const SubaccountPrivate&) = delete;
    SubaccountPrivate(SubaccountPrivate&&) = delete;
    auto operator=(const SubaccountPrivate&) -> SubaccountPrivate& = delete;
    auto operator=(SubaccountPrivate&&) -> SubaccountPrivate& = delete;

    ~SubaccountPrivate() override;

protected:
    using AddressMap = Map<Bip32Index, std::unique_ptr<crypto::Element>>;
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
    const crypto::Target target_;
    const SubaccountType type_;
    const identifier::Account id_;
    const identifier::Generic source_;
    const CString source_description_;
    const CString display_name_;
    const CString description_;
    mutable std::recursive_mutex lock_;
    mutable std::atomic<Revision> revision_;

    using SerializedActivity =
        google::protobuf::RepeatedPtrField<proto::BlockchainActivity>;
    using SerializedType = proto::BlockchainAccountData;

    static auto describe(
        const api::Session& api,
        const crypto::Target target,
        const SubaccountType type,
        const identifier::Generic& id) noexcept -> CString;

    virtual auto account_already_exists(const rLock& lock) const noexcept
        -> bool = 0;
    virtual auto save(const rLock& lock) const noexcept -> bool = 0;
    auto serialize_common(const rLock& lock, SerializedType& out) const noexcept
        -> void;

    // NOTE call only from final constructor bodies
    virtual auto init(bool existing) noexcept(false) -> void;
    virtual auto mutable_element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) -> crypto::Element& = 0;

    SubaccountPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        const identifier::Account& id,
        identifier::Generic source,
        std::string_view sourceName,
        std::string_view name) noexcept;
    SubaccountPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        const identifier::Account& id,
        identifier::Generic source,
        std::string_view sourceName,
        std::string_view name,
        const SerializedType& serialized) noexcept(false);

private:
    static constexpr auto ActivityVersion = VersionNumber{1};
    static constexpr auto BlockchainAccountDataVersion = VersionNumber{1};

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

    SubaccountPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        const identifier::Account& id,
        const Revision revision,
        identifier::Generic source,
        std::string_view sourceName,
        std::string_view name) noexcept;
};
}  // namespace opentxs::blockchain::crypto
