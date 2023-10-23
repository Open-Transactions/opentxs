// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Subaccount.hpp"  // IWYU pragma: associated

#include <BlockchainAccountData.pb.h>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/blockchain/crypto/Element.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::crypto::implementation
{
using namespace std::literals;

Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    identifier::Account&& id,
    const Revision revision,
    identifier::Account& out) noexcept
    : api_(api)
    , parent_(parent)
    , target_(parent_.Target())
    , type_(type)
    , id_(std::move(id))
    , description_(describe(api_, target_, type_, id_))
    , lock_()
    , revision_(revision)
{
    out = id_;
}

Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    identifier::Account&& id,
    identifier::Account& out) noexcept
    : Subaccount(api, parent, type, std::move(id), 0, out)
{
}

Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    const SerializedType& serialized,
    identifier::Account& out) noexcept(false)
    : Subaccount(
          api,
          parent,
          type,
          api.Factory().AccountIDFromBase58(serialized.id()),
          serialized.revision(),
          out)
{
    const auto expected = target_to_unit(target_);

    if (ClaimToUnit(translate(serialized.chain())) != expected) {
        throw std::runtime_error("Wrong account type");
    }
}

Subaccount::AddressData::AddressData(
    const api::Session& api,
    Subchain type,
    bool contact) noexcept
    : type_(type)
    , set_contact_(contact)
    , progress_(-1, block::Hash{})
    , map_()
{
}

auto Subaccount::AddressData::check_keys() const noexcept -> bool
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

auto Subaccount::Confirm(
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

auto Subaccount::describe(
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

auto Subaccount::init(bool existing) noexcept(false) -> void
{
    using opentxs::blockchain::is_supported;

    if (existing && (false == is_supported(base_chain(target_)))) {
        existing = false;
    }

    if (false == parent_.Internal().ClaimAccountID(id_, existing, this)) {
        throw std::runtime_error{
            "unable to claim subaccount id "s
                .append(id_.asBase58(api_.Crypto()))
                .append(" apparently due to an id collision")};
    }
}

auto Subaccount::PrivateKey(
    const implementation::Element& element,
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept
    -> const opentxs::crypto::asymmetric::key::EllipticCurve&
{
    static const auto blank = opentxs::crypto::asymmetric::key::EllipticCurve{};

    return blank;
}

auto Subaccount::ScanProgress(Subchain type) const noexcept -> block::Position
{
    static const auto blank = block::Position{-1, block::Hash{}};

    return blank;
}

auto Subaccount::serialize_common(
    const rLock&,
    proto::BlockchainAccountData& out) const noexcept -> void
{
    out.set_version(BlockchainAccountDataVersion);
    out.set_id(id_.asBase58(api_.Crypto()));
    out.set_revision(revision_.load());
    out.set_chain(translate(UnitToClaim(target_to_unit(target_))));
}

auto Subaccount::SetContact(
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

auto Subaccount::SetLabel(
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

auto Subaccount::Unconfirm(
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

auto Subaccount::Unreserve(const Subchain type, const Bip32Index index) noexcept
    -> bool
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

auto Subaccount::UpdateElement(
    UnallocatedVector<ReadView>& pubkeyHashes) const noexcept -> void
{
    parent_.Parent().Parent().Internal().UpdateElement(pubkeyHashes);
}

Subaccount::~Subaccount() = default;
}  // namespace opentxs::blockchain::crypto::implementation
