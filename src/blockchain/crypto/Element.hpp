// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>

#include "Proto.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainAccountData.pb.h"
#include "serialization/protobuf/BlockchainActivity.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Element final : virtual public internal::Element
{
public:
    auto Address(const blockchain::crypto::AddressStyle format) const noexcept
        -> UnallocatedCString final;
    auto Confirmed() const noexcept -> Txids final;
    auto Contact() const noexcept -> OTIdentifier final;
    auto Elements() const noexcept -> UnallocatedSet<OTData> final;
    auto elements(const rLock& lock) const noexcept -> UnallocatedSet<OTData>;
    auto ID() const noexcept -> const Identifier& final { return parent_.ID(); }
    auto IncomingTransactions() const noexcept
        -> UnallocatedSet<UnallocatedCString> final;
    auto Internal() const noexcept -> internal::Element& final
    {
        return const_cast<Element&>(*this);
    }
    auto IsAvailable(const Identifier& contact, const UnallocatedCString& memo)
        const noexcept -> Availability final;
    auto Index() const noexcept -> Bip32Index final { return index_; }
    auto Key() const noexcept -> ECKey final;
    auto KeyID() const noexcept -> crypto::Key final
    {
        return {ID().str(), subchain_, index_};
    }
    auto Label() const noexcept -> UnallocatedCString final;
    auto LastActivity() const noexcept -> Time final;
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return parent_.Parent().NymID();
    }
    auto Parent() const noexcept -> const crypto::Subaccount& final
    {
        return parent_;
    }
    auto PrivateKey(const PasswordPrompt& reason) const noexcept -> ECKey final;
    auto PubkeyHash() const noexcept -> OTData final;
    auto Serialize() const noexcept -> SerializedType final;
    auto Subchain() const noexcept -> crypto::Subchain final
    {
        return subchain_;
    }
    auto Unconfirmed() const noexcept -> Txids final;

    auto Confirm(const Txid& tx) noexcept -> bool final;
    auto Reserve(const Time time) noexcept -> bool final;
    auto SetContact(const Identifier& id) noexcept -> void final;
    auto SetLabel(const UnallocatedCString& label) noexcept -> void final;
    auto SetMetadata(
        const Identifier& contact,
        const UnallocatedCString& label) noexcept -> void final;
    auto Unconfirm(const Txid& tx, const Time time) noexcept -> bool final;
    auto Unreserve() noexcept -> bool final;

    Element(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const crypto::Subaccount& parent,
        const opentxs::blockchain::Type chain,
        const crypto::Subchain subchain,
        const Bip32Index index,
        const opentxs::crypto::key::EllipticCurve& key,
        OTIdentifier&& contact) noexcept(false);
    Element(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const crypto::Subaccount& parent,
        const opentxs::blockchain::Type chain,
        const crypto::Subchain subchain,
        const SerializedType& address) noexcept(false);
    Element(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const crypto::Subaccount& parent,
        const opentxs::blockchain::Type chain,
        const crypto::Subchain subchain,
        const SerializedType& address,
        OTIdentifier&& contact) noexcept(false);
    ~Element() final = default;

private:
    using pTxid = opentxs::blockchain::block::pTxid;
    using Transactions = boost::container::flat_set<pTxid>;

    static const VersionNumber DefaultVersion{1};

    const api::Session& api_;
    const api::crypto::Blockchain& blockchain_;
    const crypto::Subaccount& parent_;
    const opentxs::blockchain::Type chain_;
    mutable std::recursive_mutex lock_;
    const VersionNumber version_;
    const crypto::Subchain subchain_;
    const Bip32Index index_;
    UnallocatedCString label_;
    OTIdentifier contact_;
    mutable std::shared_ptr<const opentxs::crypto::key::EllipticCurve> pkey_;
    Time timestamp_;
    Transactions unconfirmed_;
    Transactions confirmed_;
    mutable std::optional<SerializedType> cached_;

    static auto instantiate(
        const api::Session& api,
        const proto::AsymmetricKey& serialized) noexcept(false)
        -> std::unique_ptr<opentxs::crypto::key::EllipticCurve>;

    auto update_element(rLock& lock) const noexcept -> void;

    Element(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const crypto::Subaccount& parent,
        const opentxs::blockchain::Type chain,
        const VersionNumber version,
        const crypto::Subchain subchain,
        const Bip32Index index,
        const UnallocatedCString label,
        OTIdentifier&& contact,
        const opentxs::crypto::key::EllipticCurve& key,
        const Time time,
        Transactions&& unconfirmed,
        Transactions&& confirmed) noexcept(false);
    Element() = delete;
};
}  // namespace opentxs::blockchain::crypto::implementation
