// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::PaymentCode

#include "blockchain/crypto/subaccount/paymentcode/Imp.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <Bip47Direction.pb.h>
#include <BlockchainAddress.pb.h>
#include <HDPath.pb.h>
#include <PaymentCode.pb.h>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "blockchain/crypto/element/Element.hpp"
#include "blockchain/crypto/subaccount/deterministic/Imp.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/blockchain/crypto/Element.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto
{
PaymentCodePrivate::PaymentCodePrivate(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const crypto::Account& parent,
    const identifier::Account& id,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const PasswordPrompt& reason) noexcept(false)
    : DeterministicPrivate(
          api,
          parent,
          SubaccountType::PaymentCode,
          id,
          local.ID(),
          source(local),
          name(remote),
          path,
          {api, internal_type_, false, external_type_, false})
    , version_(DefaultVersion)
    , outgoing_notifications_()
    , incoming_notifications_()
    , local_(local, compare_)
    , remote_(remote, compare_)
    , contact_id_(
          contacts.PaymentCodeToContact(remote_, target_to_unit(target_)))
    , self_()
{
    const auto test_path = local_.get().Internal().AddPrivateKeys(
        seed_id_, *path_.child().rbegin(), reason);

    if (false == test_path) {
        throw std::runtime_error("Invalid path or local payment code");
    }

    if (contact_id_.empty()) { throw std::runtime_error("Missing contact"); }

    if (local == remote) {
        throw std::runtime_error(
            "remote payment code is the same as local payment code");
    }

    init(reason);
    parent_.Internal().FindNym(remote_.get().ID());
}

PaymentCodePrivate::PaymentCodePrivate(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const crypto::Account& parent,
    const identifier::Account& id,
    const SerializedType& serialized,
    opentxs::PaymentCode local,
    opentxs::PaymentCode remote,
    identifier::Generic contact) noexcept(false)
    : DeterministicPrivate(
          api,
          parent,
          SubaccountType::PaymentCode,
          id,
          serialized.deterministic(),
          local.ID(),
          source(local),
          name(remote),
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
                              Self(),
                              base_chain(parent.Target()),
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
                              Self(),
                              base_chain(parent.Target()),
                              external_type_,
                              address,
                              identifier::Generic{fallback})));
              }

              return out;
          }())
    , version_(serialized.version())
    , outgoing_notifications_([&] {
        auto out =
            UnallocatedSet<opentxs::blockchain::block::TransactionHash>{};

        for (const auto& notif : serialized.outgoing().notification()) {
            if (const auto [i, _] = out.emplace(notif); i->IsNull()) {
                out.erase(i);
            }
        }

        return out;
    }())
    , incoming_notifications_([&] {
        auto out =
            UnallocatedSet<opentxs::blockchain::block::TransactionHash>{};

        for (const auto& notif : serialized.incoming().notification()) {
            if (const auto [i, _] = out.emplace(notif); i->IsNull()) {
                out.erase(i);
            }
        }

        return out;
    }())
    , local_(std::move(local), compare_)
    , remote_(std::move(remote), compare_)
    , contact_id_(
          contacts.PaymentCodeToContact(remote_, target_to_unit(target_)))
    , self_()
{
    if (contact_id_.empty()) { throw std::runtime_error("Missing contact"); }

    init(true);
    parent_.Internal().FindNym(remote_.get().ID());
}

PaymentCodePrivate::PaymentCodePrivate(
    const api::Session& api,
    const api::session::Contacts& contacts,
    const crypto::Account& parent,
    const identifier::Account& id,
    const SerializedType& serialized,
    identifier::Generic&& contact) noexcept(false)
    : PaymentCodePrivate(
          api,
          contacts,
          parent,
          id,
          serialized,
          api.Factory().Internal().Session().PaymentCode(serialized.local()),
          api.Factory().Internal().Session().PaymentCode(serialized.remote()),
          std::move(contact))
{
}

auto PaymentCodePrivate::account_already_exists(const rLock&) const noexcept
    -> bool
{
    const auto existing = api_.Storage().Internal().Bip47ChannelsByChain(
        parent_.NymID(), target_to_unit(target_));

    return existing.contains(id_);
}

auto PaymentCodePrivate::AddIncomingNotification(
    const block::TransactionHash& tx) const noexcept -> bool
{
    auto lock = rLock{lock_};

    if (incoming_notifications_.contains(tx)) { return true; }

    incoming_notifications_.emplace(tx);
    const auto out = save(lock);

    if (false == out) { incoming_notifications_.erase(tx); }

    return out;
}

auto PaymentCodePrivate::AddNotification(
    const block::TransactionHash& tx) const noexcept -> bool
{
    auto lock = rLock{lock_};

    if (outgoing_notifications_.contains(tx)) { return true; }

    outgoing_notifications_.emplace(tx);
    const auto out = save(lock);

    if (false == out) { outgoing_notifications_.erase(tx); }

    return out;
}

auto PaymentCodePrivate::has_private(
    const PasswordPrompt& reason) const noexcept -> bool
{
    const auto& key = local_.get().Key();

    if (false == key.IsValid()) {
        LogError()()("No local HD key").Flush();

        return false;
    }

    if (key.HasPrivate()) { return true; }

    return local_.get().Internal().AddPrivateKeys(
        seed_id_, *path_.child().rbegin(), reason);
}

auto PaymentCodePrivate::IncomingNotificationCount() const noexcept
    -> std::size_t
{
    auto lock = rLock{lock_};

    return incoming_notifications_.size();
}

auto PaymentCodePrivate::InitSelf(std::shared_ptr<Subaccount> me) noexcept
    -> void
{
    self_.emplace(me);
}

auto PaymentCodePrivate::name(const opentxs::PaymentCode& remote) noexcept
    -> UnallocatedCString
{
    return remote.asBase58() + " (remote)";
}

auto PaymentCodePrivate::NotificationCount() const noexcept
    -> std::pair<std::size_t, std::size_t>
{
    auto lock = rLock{lock_};

    return std::make_pair(
        incoming_notifications_.size(), outgoing_notifications_.size());
}

auto PaymentCodePrivate::OutgoingNotificationCount() const noexcept
    -> std::size_t
{
    auto lock = rLock{lock_};

    return outgoing_notifications_.size();
}

auto PaymentCodePrivate::PrivateKey(
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve
{
    if (false == has_private(reason)) {
        LogError()()("Missing private key").Flush();

        return {};
    }

    switch (type) {
        case internal_type_: {

            return local_.get().Outgoing(
                remote_.get(), index, base_chain(target_), reason);
        }
        case external_type_: {

            return local_.get().Incoming(
                remote_.get(), index, base_chain(target_), reason);
        }
        default: {
            LogError()()("Invalid subchain").Flush();

            return {};
        }
    }
}

auto PaymentCodePrivate::ReorgNotification(
    const block::TransactionHash& tx) const noexcept -> bool
{
    auto lock = rLock{lock_};

    if (false == outgoing_notifications_.contains(tx)) { return true; }

    outgoing_notifications_.erase(tx);
    const auto out = save(lock);

    if (false == out) { outgoing_notifications_.emplace(tx); }

    return out;
}

auto PaymentCodePrivate::Reserve(
    const Subchain type,
    const std::size_t batch,
    const identifier::Generic&,
    const PasswordPrompt& reason,
    const std::string_view label,
    const Time time) const noexcept -> Batch
{
    return reserve(type, batch, get_contact(), reason, label, time);
}

auto PaymentCodePrivate::RootNode(const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::HD&
{
    return local_.get().Key();
}

auto PaymentCodePrivate::save(const rLock& lock) const noexcept -> bool
{
    auto serialized = SerializedType{};
    serialized.set_version(version_);
    serialize_deterministic(lock, *serialized.mutable_deterministic());
    auto local = proto::PaymentCode{};
    if (false == local_.get().Internal().Serialize(local)) {
        LogError()()("Failed to serialize local paymentcode").Flush();

        return false;
    }
    *serialized.mutable_local() = local;
    auto remote = proto::PaymentCode{};
    if (false == remote_.get().Internal().Serialize(remote)) {
        LogError()()("Failed to serialize remote paymentcode").Flush();

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

    const bool saved =
        api_.Storage().Internal().Store(parent_.NymID(), id_, serialized);

    if (false == saved) {
        LogError()()("Failed to save PaymentCode account").Flush();

        return false;
    }

    return saved;
}

auto PaymentCodePrivate::source(const opentxs::PaymentCode& local) noexcept
    -> UnallocatedCString
{
    return local.asBase58() + " (local)";
}

PaymentCodePrivate::~PaymentCodePrivate() = default;
}  // namespace opentxs::blockchain::crypto
