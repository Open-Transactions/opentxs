// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/crypto/Subaccount.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/core/Factory.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/HD.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainAccountData.pb.h"
#include "serialization/protobuf/BlockchainActivity.pb.h"

namespace opentxs::blockchain::crypto::implementation
{
Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    OTIdentifier&& id,
    const Revision revision,
    const UnallocatedVector<Activity>& unspent,
    const UnallocatedVector<Activity>& spent,
    Identifier& out) noexcept
    : api_(api)
    , parent_(parent)
    , chain_(parent_.Chain())
    , type_(type)
    , id_(std::move(id))
    , lock_()
    , revision_(revision)
    , unspent_(convert(unspent))
    , spent_(convert(spent))
{
    out.Assign(id_);
}

Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    OTIdentifier&& id,
    Identifier& out) noexcept
    : Subaccount(api, parent, type, std::move(id), 0, {}, {}, out)
{
}

Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    const SerializedType& serialized,
    Identifier& out) noexcept(false)
    : Subaccount(
          api,
          parent,
          type,
          api.Factory().Identifier(serialized.id()),
          serialized.revision(),
          convert(serialized.unspent()),
          convert(serialized.spent()),
          out)
{
    if (UnitToBlockchain(ClaimToUnit(translate(serialized.chain()))) !=
        chain_) {
        throw std::runtime_error("Wrong account type");
    }
}

Subaccount::AddressData::AddressData(
    const api::Session& api,
    Subchain type,
    bool contact) noexcept
    : type_(type)
    , set_contact_(contact)
    , progress_(-1, block::BlankHash())
    , map_()
{
}

auto Subaccount::AssociateTransaction(
    const UnallocatedVector<Activity>& unspent,
    const UnallocatedVector<Activity>& spent,
    UnallocatedSet<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = rLock{lock_};

    if (false == check_activity(lock, unspent, contacts, reason)) {

        return false;
    }

    for (const auto& [coin, key, value] : spent) {
        process_spent(lock, coin, key, value);
    }

    for (const auto& [coin, key, value] : unspent) {
        process_unspent(lock, coin, key, value);
    }

    return save(lock);
}

auto Subaccount::Confirm(
    const Subchain type,
    const Bip32Index index,
    const Txid& tx) noexcept -> bool
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
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Subaccount::convert(Activity&& in) noexcept -> proto::BlockchainActivity
{
    const auto& [coin, key, value] = in;
    const auto& [txid, out] = coin;
    const auto& [account, chain, index] = key;
    proto::BlockchainActivity output{};
    output.set_version(ActivityVersion);
    output.set_txid(txid);
    output.set_output(out);
    value.Serialize(writer(output.mutable_amount()));
    output.set_account(account);
    output.set_subchain(static_cast<std::uint32_t>(chain));
    output.set_index(index);

    return output;
}

auto Subaccount::convert(const proto::BlockchainActivity& in) noexcept
    -> Activity
{
    Activity output{};
    auto& [coin, key, value] = output;
    auto& [txid, out] = coin;
    auto& [account, chain, index] = key;
    txid = in.txid();
    out = in.output();
    value = factory::Amount(in.amount());
    account = in.account();
    chain = static_cast<Subchain>(in.subchain());
    index = in.index();

    return output;
}

auto Subaccount::convert(const SerializedActivity& in) noexcept
    -> UnallocatedVector<Activity>
{
    auto output = UnallocatedVector<Activity>{};

    for (const auto& activity : in) { output.emplace_back(convert(activity)); }

    return output;
}

auto Subaccount::convert(const UnallocatedVector<Activity>& in) noexcept
    -> internal::ActivityMap
{
    auto output = internal::ActivityMap{};

    for (const auto& [coin, key, value] : in) {
        output.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(coin),
            std::forward_as_tuple(key, value));
    }

    return output;
}

auto Subaccount::IncomingTransactions(const Key& element) const noexcept
    -> UnallocatedSet<UnallocatedCString>
{
    auto lock = rLock{lock_};
    auto output = UnallocatedSet<UnallocatedCString>{};

    for (const auto& [coin, data] : unspent_) {
        const auto& [key, amount] = data;

        if (key == element) { output.emplace(coin.first); }
    }

    for (const auto& [coin, data] : spent_) {
        const auto& [key, amount] = data;

        if (key == element) { output.emplace(coin.first); }
    }

    return output;
}

void Subaccount::init() noexcept
{
    parent_.Internal().ClaimAccountID(id_->str(), this);
}

// Due to asynchronous blockchain scanning, spends may be discovered out of
// order compared to receipts.
void Subaccount::process_spent(
    const rLock& lock,
    const Coin& coin,
    const Key key,
    const Amount value) const noexcept
{
    auto targetValue{value};

    if (0 < unspent_.count(coin)) {
        // Normal case
        targetValue = std::max(targetValue, unspent_.at(coin).second);
        unspent_.erase(coin);
    }

    // If the spend was found before the receipt, the value is not known
    spent_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(coin),
        std::forward_as_tuple(key, targetValue));
}

void Subaccount::process_unspent(
    const rLock& lock,
    const Coin& coin,
    const Key key,
    const Amount value) const noexcept
{
    if (0 == spent_.count(coin)) {
        // Normal case
        unspent_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(coin),
            std::forward_as_tuple(key, value));
    } else {
        // Spend was discovered out of order, so correct the value now
        auto& storedValue = spent_.at(coin).second;
        storedValue = std::max(storedValue, value);
    }
}

auto Subaccount::ScanProgress(Subchain type) const noexcept -> block::Position
{
    static const auto blank = block::Position{-1, api_.Factory().Data()};

    return blank;
}

auto Subaccount::serialize_common(
    const rLock&,
    proto::BlockchainAccountData& out) const noexcept -> void
{
    out.set_version(BlockchainAccountDataVersion);
    out.set_id(id_->str());
    out.set_revision(revision_.load());
    out.set_chain(translate(UnitToClaim(BlockchainToUnit(chain_))));

    for (const auto& [coin, data] : unspent_) {
        auto converted = Activity{coin, data.first, data.second};
        *out.add_unspent() = convert(std::move(converted));
    }

    for (const auto& [coin, data] : spent_) {
        auto converted = Activity{coin, data.first, data.second};
        *out.add_spent() = convert(std::move(converted));
    }
}

auto Subaccount::SetContact(
    const Subchain type,
    const Bip32Index index,
    const Identifier& id) noexcept(false) -> bool
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
    const UnallocatedCString& label) noexcept(false) -> bool
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
    const Txid& tx,
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
}  // namespace opentxs::blockchain::crypto::implementation
