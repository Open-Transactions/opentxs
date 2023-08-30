// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/crypto/Subaccount.hpp"  // IWYU pragma: associated

#include <BlockchainAccountData.pb.h>
#include <BlockchainActivity.pb.h>
#include <algorithm>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/blockchain/crypto/Element.hpp"
#include "internal/core/Factory.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::crypto::implementation
{
Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    identifier::Account&& id,
    const Revision revision,
    const UnallocatedVector<Activity>& unspent,
    const UnallocatedVector<Activity>& spent,
    identifier::Account& out) noexcept
    : api_(api)
    , parent_(parent)
    , chain_(parent_.Chain())
    , type_(type)
    , id_(std::move(id))
    , description_(describe(api_, chain_, type_, id_))
    , lock_()
    , revision_(revision)
    , unspent_(convert(unspent))
    , spent_(convert(spent))
{
    out = id_;
}

Subaccount::Subaccount(
    const api::Session& api,
    const crypto::Account& parent,
    const SubaccountType type,
    identifier::Account&& id,
    identifier::Account& out) noexcept
    : Subaccount(api, parent, type, std::move(id), 0, {}, {}, out)
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
          convert(api, serialized.unspent()),
          convert(api, serialized.spent()),
          out)
{
    if (unit_to_blockchain(ClaimToUnit(translate(serialized.chain()))) !=
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
    , progress_(-1, block::Hash{})
    , map_()
{
}

auto Subaccount::AddressData::check_keys() const noexcept -> bool
{
    auto counter{-1};

    for (const auto& [index, element] : map_) {
        if (index != static_cast<Bip32Index>(++counter)) {
            LogError()(OT_PRETTY_CLASS())("key ")(
                index)(" present at position ")(counter)
                .Flush();

            return false;
        }
    }

    return true;
}

auto Subaccount::AssociateTransaction(
    const UnallocatedVector<Activity>& unspent,
    const UnallocatedVector<Activity>& spent,
    UnallocatedSet<identifier::Generic>& contacts,
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
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Subaccount::convert(const api::Session& api, Activity&& in) noexcept
    -> proto::BlockchainActivity
{
    const auto& [coin, key, value] = in;
    const auto& [txid, out] = coin;
    const auto& [account, chain, index] = key;
    auto output = proto::BlockchainActivity{};
    output.set_version(ActivityVersion);
    output.set_txid(txid);
    output.set_output(out);
    value.Serialize(writer(output.mutable_amount()));
    output.set_account(account.asBase58(api.Crypto()));
    output.set_subchain(static_cast<std::uint32_t>(chain));
    output.set_index(index);

    return output;
}

auto Subaccount::convert(
    const api::Session& api,
    const proto::BlockchainActivity& in) noexcept -> Activity
{
    Activity output{};
    auto& [coin, key, value] = output;
    auto& [txid, out] = coin;
    auto& [account, chain, index] = key;
    txid = in.txid();
    out = convert_to_size(in.output());
    value = factory::Amount(in.amount());
    account = api.Factory().AccountIDFromBase58(in.account());
    chain = static_cast<Subchain>(in.subchain());
    index = in.index();

    return output;
}

auto Subaccount::convert(
    const api::Session& api,
    const SerializedActivity& in) noexcept -> UnallocatedVector<Activity>
{
    auto output = UnallocatedVector<Activity>{};

    for (const auto& activity : in) {
        output.emplace_back(convert(api, activity));
    }

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

auto Subaccount::describe(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    const SubaccountType type,
    const identifier::Generic& id) noexcept -> CString
{
    // TODO c++20 use allocator
    auto out = std::stringstream{};
    out << print(chain) << ' ';
    out << print(type);
    out << " account ";
    out << id.asBase58(api.Crypto());

    return CString{} + out.str().c_str();
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
    parent_.Internal().ClaimAccountID(id_, this);
}

auto Subaccount::PrivateKey(
    const Subchain type,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept
    -> opentxs::crypto::asymmetric::key::EllipticCurve
{
    return {};
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
    out.set_chain(translate(UnitToClaim(blockchain_to_unit(chain_))));

    for (const auto& [coin, data] : unspent_) {
        auto converted = Activity{coin, data.first, data.second};
        *out.add_unspent() = convert(api_, std::move(converted));
    }

    for (const auto& [coin, data] : spent_) {
        auto converted = Activity{coin, data.first, data.second};
        *out.add_spent() = convert(api_, std::move(converted));
    }
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
