// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "blockchain/crypto/PaymentCode.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <Bip47Direction.pb.h>
#include <BlockchainAddress.pb.h>
#include <HDPath.pb.h>
#include <PaymentCode.pb.h>
#include <cstdint>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "blockchain/crypto/Deterministic.hpp"
#include "blockchain/crypto/Element.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainPCSubaccount(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const blockchain::crypto::Account& parent,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const Data& txid,
    const PasswordPrompt& reason,
    identifier::Generic& id) noexcept
    -> std::unique_ptr<blockchain::crypto::PaymentCode>
{
    using ReturnType = blockchain::crypto::implementation::PaymentCode;

    try {

        return std::make_unique<ReturnType>(
            api, contacts, parent, local, remote, path, txid, reason, id);
    } catch (const std::exception& e) {
        LogVerbose()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BlockchainPCSubaccount(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const blockchain::crypto::Account& parent,
    const proto::Bip47Channel& serialized,
    identifier::Generic& id) noexcept
    -> std::unique_ptr<blockchain::crypto::PaymentCode>
{
    using ReturnType = blockchain::crypto::implementation::PaymentCode;
    auto contact = contacts.PaymentCodeToContact(
        api.Factory().InternalSession().PaymentCode(serialized.remote()),
        parent.Chain());

    OT_ASSERT(false == contact.empty());

    try {

        return std::make_unique<ReturnType>(
            api, contacts, parent, serialized, id, std::move(contact));
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::crypto::internal
{
auto PaymentCode::GetID(
    const api::Session& api,
    const Chain chain,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote) noexcept -> identifier::Generic
{
    auto out = identifier::Generic{};
    auto preimage = api.Factory().Data();
    preimage.Assign(&chain, sizeof(chain));
    preimage.Concatenate(local.ID().Bytes());
    preimage.Concatenate(remote.ID().Bytes());

    return api.Factory().IdentifierFromPreimage(preimage.Bytes());
}
}  // namespace opentxs::blockchain::crypto::internal

namespace opentxs::blockchain::crypto::implementation
{
PaymentCode::PaymentCode(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const crypto::Account& parent,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const opentxs::blockchain::block::Txid& txid,
    const PasswordPrompt& reason,
    identifier::Generic& id) noexcept(false)
    : Deterministic(
          api,
          parent,
          SubaccountType::PaymentCode,
          internal::PaymentCode::GetID(api, parent.Chain(), local, remote),
          path,
          {api, internal_type_, false, external_type_, false},
          id)
    , version_(DefaultVersion)
    , outgoing_notifications_()
    , incoming_notifications_([&] {
        auto out = UnallocatedSet<opentxs::blockchain::block::pTxid>{};

        if (false == txid.empty()) { out.emplace(txid); }

        return out;
    }())
    , local_(local, compare_)
    , remote_(remote, compare_)
    , contact_id_(contacts.PaymentCodeToContact(remote_, chain_))
{
    const auto test_path = [&] {
        auto seed{path_.root()};

        return local_.get().Internal().AddPrivateKeys(
            seed, *path_.child().rbegin(), reason);
    };

    if (false == test_path()) {
        throw std::runtime_error("Invalid path or local payment code");
    }

    if (contact_id_.empty()) { throw std::runtime_error("Missing contact"); }

    init(reason);
    parent_.Internal().FindNym(remote_.get().ID());
}

PaymentCode::PaymentCode(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const crypto::Account& parent,
    const SerializedType& serialized,
    identifier::Generic& id,
    identifier::Generic&& contact) noexcept(false)
    : Deterministic(
          api,
          parent,
          SubaccountType::PaymentCode,
          serialized.deterministic(),
          serialized.outgoing().address().size(),
          serialized.incoming().address().size(),
          [&, fallback = std::move(contact)] {
              auto out =
                  ChainData{api, internal_type_, false, external_type_, false};
              auto& internal = out.internal_.map_;
              auto& external = out.external_.map_;

              for (const auto& address : serialized.outgoing().address()) {
                  internal.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(address.index()),
                      std::forward_as_tuple(
                          std::make_unique<implementation::Element>(
                              api,
                              parent.Parent().Parent(),
                              *this,
                              parent.Chain(),
                              internal_type_,
                              address,
                              identifier::Generic{fallback})));
              }

              for (const auto& address : serialized.incoming().address()) {
                  external.emplace(
                      std::piecewise_construct,
                      std::forward_as_tuple(address.index()),
                      std::forward_as_tuple(
                          std::make_unique<implementation::Element>(
                              api,
                              parent.Parent().Parent(),
                              *this,
                              parent.Chain(),
                              external_type_,
                              address,
                              identifier::Generic{fallback})));
              }

              return out;
          }(),
          id)
    , version_(serialized.version())
    , outgoing_notifications_([&] {
        auto out = UnallocatedSet<opentxs::blockchain::block::pTxid>{};

        for (const auto& notif : serialized.outgoing().notification()) {
            out.emplace(api_.Factory().DataFromBytes(notif));
        }

        return out;
    }())
    , incoming_notifications_([&] {
        auto out = UnallocatedSet<opentxs::blockchain::block::pTxid>{};

        for (const auto& notif : serialized.incoming().notification()) {
            out.emplace(api_.Factory().DataFromBytes(notif));
        }

        return out;
    }())
    , local_(
          api_.Factory().InternalSession().PaymentCode(serialized.local()),
          compare_)
    , remote_(
          api_.Factory().InternalSession().PaymentCode(serialized.remote()),
          compare_)
    , contact_id_(contacts.PaymentCodeToContact(remote_, chain_))
{
    if (contact_id_.empty()) { throw std::runtime_error("Missing contact"); }

    init();
    parent_.Internal().FindNym(remote_.get().ID());
}

auto PaymentCode::account_already_exists(const rLock&) const noexcept -> bool
{
    const auto existing = api_.Storage().Bip47ChannelsByChain(
        parent_.NymID(), BlockchainToUnit(chain_));

    return 0 < existing.count(id_);
}

auto PaymentCode::AddNotification(const Txid& tx) const noexcept -> bool
{
    auto lock = rLock{lock_};

    if (0 < outgoing_notifications_.count(tx)) { return true; }

    outgoing_notifications_.emplace(tx);
    const auto out = save(lock);

    if (false == out) { outgoing_notifications_.erase(tx); }

    return out;
}

auto PaymentCode::has_private(const PasswordPrompt& reason) const noexcept
    -> bool
{
    const auto& key = local_.get().Key();

    if (false == key.IsValid()) {
        LogError()(OT_PRETTY_CLASS())("No local HD key").Flush();

        return false;
    }

    if (key.HasPrivate()) { return true; }

    auto seed{path_.root()};

    return local_.get().Internal().AddPrivateKeys(
        seed, *path_.child().rbegin(), reason);
}

auto PaymentCode::IsNotified() const noexcept -> bool
{
    auto lock = rLock{lock_};

    return 0 < outgoing_notifications_.size();
}

auto PaymentCode::PrivateKey(
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept
    -> opentxs::crypto::asymmetric::key::EllipticCurve
{
    if (false == has_private(reason)) {
        LogError()(OT_PRETTY_CLASS())("Missing private key").Flush();

        return {};
    }

    switch (type) {
        case internal_type_: {
            return local_.get().Outgoing(remote_, index, chain_, reason);
        }
        case external_type_: {
            return local_.get().Incoming(remote_, index, chain_, reason);
        }
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid subchain").Flush();

            return {};
        }
    }
}

auto PaymentCode::ReorgNotification(const Txid& tx) const noexcept -> bool
{
    auto lock = rLock{lock_};

    if (0 == outgoing_notifications_.count(tx)) { return true; }

    outgoing_notifications_.erase(tx);
    const auto out = save(lock);

    if (false == out) { outgoing_notifications_.emplace(tx); }

    return out;
}

auto PaymentCode::Reserve(
    const Subchain type,
    const std::size_t batch,
    const identifier::Generic&,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> Batch
{
    return Deterministic::Reserve(
        type, batch, get_contact(), reason, label, time);
}

auto PaymentCode::RootNode(const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::HD&
{
    return local_.get().Key();
}

auto PaymentCode::save(const rLock& lock) const noexcept -> bool
{
    auto serialized = SerializedType{};
    serialized.set_version(version_);
    serialize_deterministic(lock, *serialized.mutable_deterministic());
    auto local = proto::PaymentCode{};
    if (false == local_.get().Internal().Serialize(local)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize local paymentcode")
            .Flush();

        return false;
    }
    *serialized.mutable_local() = local;
    auto remote = proto::PaymentCode{};
    if (false == remote_.get().Internal().Serialize(remote)) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize remote paymentcode")
            .Flush();

        return false;
    }
    *serialized.mutable_remote() = remote;

    {
        auto& dir = *serialized.mutable_incoming();
        dir.set_version(Bip47DirectionVersion);

        for (const auto& txid : incoming_notifications_) {
            dir.add_notification(
                static_cast<const char*>(txid.data()), txid.size());
        }

        for (const auto& [index, address] : data_.external_.map_) {
            *dir.add_address() = address->Internal().Serialize();
        }
    }

    {
        auto& dir = *serialized.mutable_outgoing();
        dir.set_version(Bip47DirectionVersion);

        for (const auto& txid : outgoing_notifications_) {
            dir.add_notification(
                static_cast<const char*>(txid.data()), txid.size());
        }

        for (const auto& [index, address] : data_.internal_.map_) {
            *dir.add_address() = address->Internal().Serialize();
        }
    }

    const bool saved = api_.Storage().Store(parent_.NymID(), id_, serialized);

    if (false == saved) {
        LogError()(OT_PRETTY_CLASS())("Failed to save PaymentCode account")
            .Flush();

        return false;
    }

    return saved;
}

PaymentCode::~PaymentCode() = default;
}  // namespace opentxs::blockchain::crypto::implementation
