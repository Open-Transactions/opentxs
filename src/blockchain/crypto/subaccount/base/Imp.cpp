// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/subaccount/base/Imp.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainAccountData.pb.h>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/crypto/Element.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto
{
using namespace std::literals;

SubaccountPrivate::SubaccountPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    const identifier::Account& id,
    const Revision revision,
    identifier::Generic source,
    std::string_view sourceName,
    std::string_view name) noexcept
    : api_(api)
    , parent_(parent)
    , target_(parent_.Target())
    , type_(type)
    , id_(id)
    , source_(std::move(source))
    , source_description_(sourceName)
    , display_name_(name)
    , description_(describe(api_, target_, type_, id_))
    , lock_()
    , revision_(revision)
{
    const auto rc = api_.Crypto().Blockchain().Internal().RegisterSubaccount(
        type_,
        base_chain(target_),  // TODO
        parent_.NymID(),
        parent_.AccountID(),
        id_);

    if (rc) {
        LogTrace()()("registered ")(Describe())(" in internal database")
            .Flush();
    }
}

SubaccountPrivate::SubaccountPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    const identifier::Account& id,
    identifier::Generic source,
    std::string_view sourceName,
    std::string_view name) noexcept
    : SubaccountPrivate(
          api,
          parent,
          type,
          id,
          0,
          std::move(source),
          sourceName,
          name)
{
}

SubaccountPrivate::SubaccountPrivate(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    const identifier::Account& id,
    identifier::Generic source,
    std::string_view sourceName,
    std::string_view name,
    const SerializedType& serialized) noexcept(false)
    : SubaccountPrivate(
          api,
          parent,
          type,
          id,
          serialized.revision(),
          std::move(source),
          sourceName,
          name)
{
    const auto expected = target_to_unit(target_);

    if (ClaimToUnit(translate(serialized.chain())) != expected) {
        throw std::runtime_error("Wrong account type");
    }
}

SubaccountPrivate::AddressData::AddressData(
    const api::Session& api,
    Subchain type,
    bool contact) noexcept
    : type_(type)
    , set_contact_(contact)
    , progress_(-1, block::Hash{})
    , map_()
{
}

auto SubaccountPrivate::AddressData::check_keys() const noexcept -> bool
{
    auto counter{-1};

    for (const auto& [index, element] : map_) {
        if (index != static_cast<Bip32Index>(++counter)) {
            LogError()()("key ")(index)(" present at position ")(counter)
                .Flush();

            return false;
        }
    }

    return true;
}

auto SubaccountPrivate::Confirm(
    const Subchain type,
    const Bip32Index index,
    const block::TransactionHash& tx) noexcept -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);

        if (element.Internal().Confirm(tx)) {
            confirm(lock, type, index);

            return save(lock);
        } else {

            return false;
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto SubaccountPrivate::describe(
    const api::Session& api,
    const crypto::Target target,
    const SubaccountType type,
    const identifier::Generic& id) noexcept -> CString
{
    // TODO c++20 use allocator
    auto out = std::stringstream{};
    out << print(target) << ' ';
    out << print(type);
    out << " account ";
    out << id.asBase58(api.Crypto());

    return CString{} + out.str().c_str();
}

auto SubaccountPrivate::init(bool) noexcept(false) -> void {}

auto SubaccountPrivate::serialize_common(
    const rLock&,
    protobuf::BlockchainAccountData& out) const noexcept -> void
{
    out.set_version(BlockchainAccountDataVersion);
    out.set_id(id_.asBase58(api_.Crypto()));
    out.set_revision(revision_.load());
    out.set_chain(translate(UnitToClaim(target_to_unit(target_))));
}

auto SubaccountPrivate::SetContact(
    const Subchain type,
    const Bip32Index index,
    const identifier::Generic& id) noexcept(false) -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);
        element.Internal().SetContact(id);

        return save(lock);
    } catch (...) {
        return false;
    }
}

auto SubaccountPrivate::SetLabel(
    const Subchain type,
    const Bip32Index index,
    const std::string_view label) noexcept(false) -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);
        element.Internal().SetLabel(label);

        return save(lock);
    } catch (...) {
        return false;
    }
}

auto SubaccountPrivate::Unconfirm(
    const Subchain type,
    const Bip32Index index,
    const block::TransactionHash& tx,
    const Time time) noexcept -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);

        if (element.Internal().Unconfirm(tx, time)) {
            unconfirm(lock, type, index);

            return save(lock);
        } else {

            return false;
        }
    } catch (...) {
        return false;
    }
}

auto SubaccountPrivate::Unreserve(
    const Subchain type,
    const Bip32Index index) noexcept -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);

        if (element.Internal().Unreserve()) {

            return save(lock);
        } else {

            return false;
        }
    } catch (...) {
        return false;
    }
}

auto SubaccountPrivate::UpdateElement(
    UnallocatedVector<ReadView>& pubkeyHashes) const noexcept -> void
{
    parent_.Parent().Parent().Internal().UpdateElement(pubkeyHashes);
}

SubaccountPrivate::~SubaccountPrivate() = default;
}  // namespace opentxs::blockchain::crypto
