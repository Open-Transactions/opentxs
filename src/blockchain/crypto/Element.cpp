// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Element.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <BlockchainAddress.pb.h>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <chrono>
#include <compare>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Asymmetric.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::crypto::implementation
{
Element::Element(
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
    Transactions&& confirmed) noexcept(false)
    : api_(api)
    , blockchain_(blockchain)
    , parent_(parent)
    , chain_(chain)
    , version_(version)
    , subchain_(subchain)
    , index_(index)
    , key_(key)
    , data_([&] {
        if (false == key.IsValid()) {
            throw std::runtime_error("No key provided");
        }

        const auto pc = (Subchain::Incoming == subchain_) ||
                        (Subchain::Outgoing == subchain_);

        if (pc && contact.empty()) {
            throw std::runtime_error("Missing contact");
        }

        return ElementPrivate{
            label,
            std::move(contact),
            time,
            std::move(unconfirmed),
            std::move(confirmed),
            std::nullopt,
            std::nullopt};
    }())
{
    if (Subchain::Error == subchain_) {
        throw std::runtime_error("Invalid subchain");
    }
}

Element::Element(
    const api::Session& api,
    const api::crypto::Blockchain& blockchain,
    const crypto::Subaccount& parent,
    const opentxs::blockchain::Type chain,
    const crypto::Subchain subchain,
    const Bip32Index index,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    identifier::Generic&& contact) noexcept(false)
    : Element(
          api,
          blockchain,
          parent,
          chain,
          DefaultVersion,
          subchain,
          index,
          "",
          std::move(contact),
          key,
          {},
          {},
          {})
{
}

Element::Element(
    const api::Session& api,
    const api::crypto::Blockchain& blockchain,
    const crypto::Subaccount& parent,
    const opentxs::blockchain::Type chain,
    const crypto::Subchain subchain,
    const SerializedType& address,
    identifier::Generic&& contact) noexcept(false)
    : Element(
          api,
          blockchain,
          parent,
          chain,
          address.version(),
          subchain,
          address.index(),
          address.label(),
          std::move(contact),
          instantiate(api, address.key()),
          convert_stime(address.modified()),
          [&] {
              auto out = Transactions{};

              for (const auto& txid : address.unconfirmed()) {
                  out.emplace(txid);
              }

              return out;
          }(),
          [&] {
              auto out = Transactions{};

              for (const auto& txid : address.confirmed()) {
                  out.emplace(txid);
              }

              return out;
          }())
{
    data_.lock()->cached_ = address;
}

Element::Element(
    const api::Session& api,
    const api::crypto::Blockchain& blockchain,
    const crypto::Subaccount& parent,
    const opentxs::blockchain::Type chain,
    const crypto::Subchain subchain,
    const SerializedType& address) noexcept(false)
    : Element(
          api,
          blockchain,
          parent,
          chain,
          subchain,
          address,
          api.Factory().IdentifierFromBase58(address.contact()))
{
}

auto Element::Address(const blockchain::crypto::AddressStyle format)
    const noexcept -> UnallocatedCString
{
    return blockchain_.CalculateAddress(
        chain_, format, api_.Factory().DataFromBytes(key_.PublicKey()));
}

auto Element::Confirmed() const noexcept -> Txids
{
    auto output = Txids{};
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    std::ranges::copy(data.confirmed_, std::back_inserter(output));

    return output;
}

auto Element::Confirm(const block::TransactionHash& tx) noexcept -> bool
{
    if (tx.empty()) {
        LogError()(OT_PRETTY_CLASS())("invalid txid").Flush();

        return false;
    }

    auto handle = data_.lock();
    auto& data = *handle;
    data.unconfirmed_.erase(tx);
    data.confirmed_.emplace(tx);
    data.timestamp_ = Clock::now();
    data.cached_ = std::nullopt;

    return true;
}

auto Element::Contact() const noexcept -> identifier::Generic
{
    return data_.lock_shared()->contact_;
}

auto Element::Elements() const noexcept -> UnallocatedSet<ByteArray>
{
    auto output = UnallocatedSet<ByteArray>{};
    auto pubkey = api_.Factory().DataFromBytes(key_.PublicKey());

    try {
        output.emplace(blockchain_.Internal().PubkeyHash(chain_, pubkey));
    } catch (...) {
        OT_FAIL;
    }

    return output;
}

auto Element::instantiate(
    const api::Session& api,
    const proto::AsymmetricKey& serialized) noexcept(false)
    -> opentxs::crypto::asymmetric::key::EllipticCurve
{
    auto output =
        api.Crypto().Asymmetric().Internal().InstantiateECKey(serialized);

    if (false == output.IsValid()) {
        throw std::runtime_error("Failed to instantiate key");
    }

    return output;
}

auto Element::IsAvailable(
    const identifier::Generic& contact,
    const std::string_view memo) const noexcept -> Availability
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    if (0 < data.confirmed_.size()) { return Availability::Used; }

    const auto age = std::chrono::duration_cast<std::chrono::hours>(
        Clock::now() - data.timestamp_);
    constexpr auto unconfirmedLimit = std::chrono::hours{24 * 7};
    constexpr auto reservedLimit = std::chrono::hours{24 * 2};
    const auto haveMetadata =
        ((false == data.contact_.empty()) || (false == data.label_.empty()));
    const auto match =
        haveMetadata && (data.contact_ == contact) && (data.label_ == memo);

    if (age > unconfirmedLimit) {
        if (match) {

            return Availability::Reissue;
        } else if (haveMetadata) {

            return Availability::MetadataConflict;
        } else if (0 < data.unconfirmed_.size()) {

            return Availability::StaleUnconfirmed;
        } else {

            return Availability::NeverUsed;
        }
    } else if (age > reservedLimit) {
        if (0 < data.unconfirmed_.size()) {

            return Availability::Reserved;
        } else if (match) {

            return Availability::Reissue;
        } else if (haveMetadata) {

            return Availability::MetadataConflict;
        } else {

            return Availability::NeverUsed;
        }
    } else {

        return Availability::Reserved;
    }
}

auto Element::Key() const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    return key_;
}

auto Element::Label() const noexcept -> UnallocatedCString
{
    return data_.lock_shared()->label_;
}

auto Element::LastActivity() const noexcept -> Time
{
    return data_.lock_shared()->timestamp_;
}

auto Element::PrivateKey(const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    if (key_.HasPrivate()) { return key_; }

    {
        auto handle = data_.lock_shared();
        const auto& data = *handle;

        if (data.private_key_.has_value()) { return *data.private_key_; }
    }

    return parent_.Internal().PrivateKey(*this, subchain_, index_, reason);
}

auto Element::PubkeyHash() const noexcept -> ByteArray
{
    return blockchain_.Internal().PubkeyHash(
        chain_, ByteArray{key_.PublicKey()});
}

auto Element::Reserve(const Time time) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;
    data.timestamp_ = time;
    data.cached_ = std::nullopt;

    return true;
}

auto Element::Serialize() const noexcept -> Element::SerializedType
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (false == data.cached_.has_value()) {
        const auto key = [&] {
            auto serialized = proto::AsymmetricKey{};

            if (key_.HasPrivate()) {
                key_.asPublic().Internal().Serialize(serialized);
            } else {
                key_.Internal().Serialize(serialized);
            }

            return serialized;
        }();

        auto& output = data.cached_.emplace();
        output.set_version(
            (DefaultVersion > version_) ? DefaultVersion : version_);
        output.set_index(index_);
        output.set_label(data.label_);
        output.set_contact(data.contact_.asBase58(api_.Crypto()));
        *output.mutable_key() = key;
        output.set_modified(Clock::to_time_t(data.timestamp_));

        for (const auto& txid : data.unconfirmed_) {
            output.add_unconfirmed(UnallocatedCString{txid.Bytes()});
        }

        for (const auto& txid : data.confirmed_) {
            output.add_confirmed(UnallocatedCString{txid.Bytes()});
        }
    }

    return data.cached_.value();
}

auto Element::SetContact(const identifier::Generic& contact) noexcept -> void
{
    const auto pc =
        (Subchain::Incoming == subchain_) || (Subchain::Outgoing == subchain_);

    if (pc) { return; }

    {
        auto handle = data_.lock();
        auto& data = *handle;

        if (data.contact_ == contact) { return; }

        data.contact_ = contact;
        data.cached_ = std::nullopt;
    }

    update_element();
}

auto Element::SetLabel(const std::string_view label) noexcept -> void
{
    {
        auto handle = data_.lock();
        auto& data = *handle;

        if (data.label_ == label) { return; }

        data.label_ = label;
        data.cached_ = std::nullopt;
    }

    update_element();
}

auto Element::SetMetadata(
    const identifier::Generic& contact,
    const std::string_view label) noexcept -> void
{
    const auto pc =
        (Subchain::Incoming == subchain_) || (Subchain::Outgoing == subchain_);

    {
        auto handle = data_.lock();
        auto& data = *handle;

        if (false == pc) { data.contact_ = contact; }

        data.label_ = label;
        data.cached_ = std::nullopt;
    }

    update_element();
}

auto Element::Unconfirm(
    const block::TransactionHash& tx,
    const Time time) noexcept -> bool
{
    if (tx.empty()) { return false; }

    auto handle = data_.lock();
    auto& data = *handle;
    data.confirmed_.erase(tx);
    data.unconfirmed_.emplace(tx);
    data.timestamp_ = time;
    data.cached_ = std::nullopt;

    return true;
}

auto Element::Unconfirmed() const noexcept -> Txids
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    auto output = Txids{};
    std::ranges::copy(data.unconfirmed_, std::back_inserter(output));

    return output;
}

auto Element::Unreserve() noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    if ((0u < data.confirmed_.size()) || (0u < data.confirmed_.size())) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "element is already associated with transactions")
            .Flush();

        return false;
    }

    data.timestamp_ = {};
    data.label_ = {};
    data.contact_ = identifier::Generic{};
    data.cached_ = std::nullopt;

    return true;
}

auto Element::update_element() const noexcept -> void
{
    const auto elements = Elements();
    auto hashes = UnallocatedVector<ReadView>{};
    std::ranges::transform(
        elements, std::back_inserter(hashes), [](const auto& in) -> auto {
            return in.Bytes();
        });
    parent_.Internal().UpdateElement(hashes);
}
}  // namespace opentxs::blockchain::crypto::implementation
