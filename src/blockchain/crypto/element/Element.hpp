// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainAddress.pb.h>
#include <boost/container/flat_set.hpp>
#include <cs_shared_guarded.h>
#include <functional>
#include <optional>
#include <shared_mutex>
#include <string_view>

#include "internal/blockchain/crypto/Element.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class DeterministicPrivate;
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class AsymmetricKey;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
using Txid = opentxs::blockchain::block::TransactionHash;
using Transactions = boost::container::flat_set<Txid>;

struct ElementPrivate {
    UnallocatedCString label_{};
    identifier::Generic contact_{};
    Time timestamp_{};
    Transactions unconfirmed_{};
    Transactions confirmed_{};
    std::optional<internal::Element::SerializedType> cached_{};
    std::optional<opentxs::crypto::asymmetric::key::EllipticCurve>
        private_key_{};
};
}  // namespace opentxs::blockchain::crypto

namespace opentxs::blockchain::crypto::implementation
{
class Element final : virtual public internal::Element
{
public:
    auto Address(const blockchain::crypto::AddressStyle format) const noexcept
        -> UnallocatedCString final;
    auto Confirmed() const noexcept -> Txids final;
    auto Contact() const noexcept -> identifier::Generic final;
    auto Elements() const noexcept -> UnallocatedSet<ByteArray> final;
    auto ID() const noexcept -> const identifier::Account& final
    {
        return parent_.ID();
    }
    auto Internal() const noexcept -> internal::Element& final
    {
        return const_cast<Element&>(*this);
    }
    auto IsAvailable(
        const identifier::Generic& contact,
        const std::string_view memo) const noexcept -> Availability final;
    auto Index() const noexcept -> Bip32Index final { return index_; }
    auto Key() const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& final;
    auto KeyID() const noexcept -> crypto::Key final
    {
        return {ID(), subchain_, index_};
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
    auto PrivateKey(const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& final;
    auto PubkeyHash() const noexcept -> ByteArray final;
    auto Serialize(bool withPrivate) const noexcept -> SerializedType final;
    auto Subchain() const noexcept -> crypto::Subchain final
    {
        return subchain_;
    }
    auto Unconfirmed() const noexcept -> Txids final;

    auto Confirm(const block::TransactionHash& tx) noexcept -> bool final;
    auto Reserve(const Time time) noexcept -> bool final;
    auto SetContact(const identifier::Generic& id) noexcept -> void final;
    auto SetLabel(const std::string_view label) noexcept -> void final;
    auto SetMetadata(
        const identifier::Generic& contact,
        const std::string_view label) noexcept -> void final;
    auto Unconfirm(const block::TransactionHash& tx, const Time time) noexcept
        -> bool final;
    auto Unreserve() noexcept -> bool final;

    Element(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const crypto::Subaccount& parent,
        const opentxs::blockchain::Type chain,
        const crypto::Subchain subchain,
        const Bip32Index index,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        identifier::Generic&& contact) noexcept(false);
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
        identifier::Generic&& contact) noexcept(false);
    Element() = delete;

    ~Element() final = default;

private:
    friend DeterministicPrivate;

    using Data = libguarded::shared_guarded<ElementPrivate, std::shared_mutex>;

    static const VersionNumber DefaultVersion{1};

    const api::Session& api_;
    const api::crypto::Blockchain& blockchain_;
    const crypto::Subaccount& parent_;
    const opentxs::blockchain::Type chain_;
    const VersionNumber version_;
    const crypto::Subchain subchain_;
    const Bip32Index index_;
    const opentxs::crypto::asymmetric::key::EllipticCurve key_;
    mutable Data data_;

    static auto instantiate(
        const api::Session& api,
        const proto::AsymmetricKey& serialized) noexcept(false)
        -> opentxs::crypto::asymmetric::key::EllipticCurve;

    auto update_element() const noexcept -> void;

    Element(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        const crypto::Subaccount& parent,
        const opentxs::blockchain::Type chain,
        const VersionNumber version,
        const crypto::Subchain subchain,
        const Bip32Index index,
        const UnallocatedCString label,
        identifier::Generic&& contact,
        const opentxs::crypto::asymmetric::key::EllipticCurve& key,
        const Time time,
        Transactions&& unconfirmed,
        Transactions&& confirmed) noexcept(false);
};
}  // namespace opentxs::blockchain::crypto::implementation
