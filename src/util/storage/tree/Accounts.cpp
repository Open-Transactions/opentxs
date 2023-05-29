// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Accounts.hpp"  // IWYU pragma: associated

#include <StorageAccountIndex.pb.h>
#include <StorageAccounts.pb.h>
#include <StorageEnums.pb.h>
#include <StorageIDList.pb.h>
#include <StorageItemHash.pb.h>
#include <memory>
#include <utility>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageAccounts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"

#define EXTRACT_SET_BY_VALUE(index, value)                                     \
    try {                                                                      \
                                                                               \
        return index.at(value);                                                \
                                                                               \
    } catch (...) {                                                            \
                                                                               \
        return {};                                                             \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define EXTRACT_SET_BY_ID(index, id) EXTRACT_SET_BY_VALUE(index, id)

#define EXTRACT_FIELD(field)                                                   \
    {                                                                          \
        Lock lock(write_lock_);                                                \
                                                                               \
        return std::get<field>(get_account_data(lock, id));                    \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define SERIALIZE_INDEX(index, field)                                          \
    for (const auto& [id, accounts] : index) {                                 \
        if (id.empty()) { continue; }                                          \
                                                                               \
        auto& listProto = *serialized.add_##field();                           \
        listProto.set_version(INDEX_VERSION);                                  \
        listProto.set_id(id.asBase58(crypto_));                                \
                                                                               \
        for (const auto& accountID : accounts) {                               \
            if (accountID.empty()) { continue; }                               \
                                                                               \
            listProto.add_list(accountID.asBase58(crypto_));                   \
        }                                                                      \
                                                                               \
        if (0 == listProto.list_size()) {                                      \
            serialized.mutable_##field()->RemoveLast();                        \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define DESERIALIZE_INDEX(field, index, position, factory)                     \
    for (const auto& it : serialized->field()) {                               \
        const auto id = factory_.factory(it.id());                             \
                                                                               \
        auto& map = index[id];                                                 \
                                                                               \
        for (const auto& account : it.list()) {                                \
            const auto accountID = factory_.AccountIDFromBase58(account);      \
                                                                               \
            map.emplace(accountID);                                            \
            std::get<position>(get_account_data(lock, accountID)) = id;        \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define ACCOUNT_VERSION 1
#define INDEX_VERSION 1

namespace opentxs::storage
{
Accounts::Accounts(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash)
    : Node(crypto, factory, storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(ACCOUNT_VERSION);
    }
}

auto Accounts::AccountContract(const identifier::Account& id) const
    -> identifier::UnitDefinition
{
    EXTRACT_FIELD(4);
}

auto Accounts::AccountIssuer(const identifier::Account& id) const
    -> identifier::Nym
{
    EXTRACT_FIELD(2);
}

auto Accounts::AccountOwner(const identifier::Account& id) const
    -> identifier::Nym
{
    EXTRACT_FIELD(0);
}

auto Accounts::AccountServer(const identifier::Account& id) const
    -> identifier::Notary
{
    EXTRACT_FIELD(3);
}

auto Accounts::AccountSigner(const identifier::Account& id) const
    -> identifier::Nym
{
    EXTRACT_FIELD(1);
}

auto Accounts::AccountUnit(const identifier::Account& id) const -> UnitType
{
    EXTRACT_FIELD(5);
}

auto Accounts::AccountsByContract(const identifier::UnitDefinition& contract)
    const -> UnallocatedSet<identifier::Account>
{
    EXTRACT_SET_BY_ID(contract_index_, contract);
}

auto Accounts::AccountsByIssuer(const identifier::Nym& issuerNym) const
    -> UnallocatedSet<identifier::Account>
{
    EXTRACT_SET_BY_ID(issuer_index_, issuerNym);
}

auto Accounts::AccountsByOwner(const identifier::Nym& ownerNym) const
    -> UnallocatedSet<identifier::Account>
{
    EXTRACT_SET_BY_ID(owner_index_, ownerNym);
}

auto Accounts::AccountsByServer(const identifier::Notary& server) const
    -> UnallocatedSet<identifier::Account>
{
    EXTRACT_SET_BY_ID(server_index_, server);
}

auto Accounts::AccountsByUnit(const UnitType unit) const
    -> UnallocatedSet<identifier::Account>
{
    EXTRACT_SET_BY_VALUE(unit_index_, unit);
}

template <typename A, typename M, typename I>
auto Accounts::add_set_index(
    const api::Crypto& crypto,
    const identifier::Account& accountID,
    const A& argID,
    M& mapID,
    I& index) -> bool
{
    if (mapID.empty()) {
        index[argID].emplace(accountID);
        mapID = argID;
    } else {
        if (mapID != argID) {
            LogError()(OT_PRETTY_STATIC(Accounts))("Provided index id (")(
                argID, crypto)(") for account ")(accountID, crypto)(
                " does not match existing index id ")(mapID, crypto)
                .Flush();

            return false;
        }

        OT_ASSERT(1 == index.at(argID).count(accountID));
    }

    return true;
}

auto Accounts::Alias(const UnallocatedCString& id) const -> UnallocatedCString
{
    return get_alias(id);
}

auto Accounts::check_update_account(
    const Lock& lock,
    const identifier::Account& accountID,
    const identifier::Nym& ownerNym,
    const identifier::Nym& signerNym,
    const identifier::Nym& issuerNym,
    const identifier::Notary& server,
    const identifier::UnitDefinition& contract,
    const UnitType unit) -> bool
{
    if (accountID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid account ID.").Flush();

        return false;
    }

    if (ownerNym.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid owner nym ID.").Flush();

        return false;
    }

    if (signerNym.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid signer nym ID.").Flush();

        return false;
    }

    if (issuerNym.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid issuer nym ID.").Flush();

        return false;
    }

    if (server.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid server ID.").Flush();

        return false;
    }

    if (contract.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid unit ID.").Flush();

        return false;
    }

    OT_ASSERT(verify_write_lock(lock));

    auto& [mapOwner, mapSigner, mapIssuer, mapServer, mapContract, mapUnit] =
        get_account_data(lock, accountID);

    if (!add_set_index(crypto_, accountID, ownerNym, mapOwner, owner_index_)) {

        return false;
    }

    if (!add_set_index(
            crypto_, accountID, signerNym, mapSigner, signer_index_)) {

        return false;
    }

    if (!add_set_index(
            crypto_, accountID, issuerNym, mapIssuer, issuer_index_)) {

        return false;
    }

    if (!add_set_index(crypto_, accountID, server, mapServer, server_index_)) {

        return false;
    }

    if (!add_set_index(
            crypto_, accountID, contract, mapContract, contract_index_)) {
        return false;
    }

    if (UnitType::Unknown != unit) {
        mapUnit = unit;
        unit_index_[unit].emplace(accountID);
    }

    return true;
}

auto Accounts::Delete(const UnallocatedCString& id) -> bool
{
    Lock lock(write_lock_);
    const auto accountID = factory_.AccountIDFromBase58(id);
    auto it = account_data_.find(accountID);

    if (account_data_.end() != it) {
        const auto& [owner, signer, issuer, server, contract, unit] =
            it->second;
        erase(accountID, owner, owner_index_);
        erase(accountID, signer, signer_index_);
        erase(accountID, issuer, issuer_index_);
        erase(accountID, server, server_index_);
        erase(accountID, contract, contract_index_);
        erase(accountID, unit, unit_index_);
        account_data_.erase(it);
    }

    return delete_item(lock, id);
}

auto Accounts::get_account_data(
    const Lock& lock,
    const identifier::Account& accountID) const -> Accounts::AccountData&
{
    OT_ASSERT(verify_write_lock(lock));

    auto data = account_data_.find(accountID);

    if (account_data_.end() == data) {
        auto [output, added] = account_data_.emplace(
            accountID, AccountData{{}, {}, {}, {}, {}, UnitType::Unknown});

        OT_ASSERT(added);

        return output->second;
    }

    return data->second;
}

void Accounts::init(const UnallocatedCString& hash)
{
    Lock lock(write_lock_);
    std::shared_ptr<proto::StorageAccounts> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load account index file.")
            .Flush();
        OT_FAIL;
    }

    init_version(ACCOUNT_VERSION, *serialized);

    for (const auto& it : serialized->account()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    DESERIALIZE_INDEX(owner, owner_index_, 0, NymIDFromBase58);
    DESERIALIZE_INDEX(signer, signer_index_, 1, NymIDFromBase58);
    DESERIALIZE_INDEX(issuer, issuer_index_, 2, NymIDFromBase58);
    DESERIALIZE_INDEX(server, server_index_, 3, NotaryIDFromBase58);
    DESERIALIZE_INDEX(unit, contract_index_, 4, UnitIDFromBase58);

    for (const auto& it : serialized->index()) {
        const auto unit = it.type();
        const auto type = ClaimToUnit(translate(unit));
        auto& map = unit_index_[type];

        for (const auto& account : it.account()) {
            const auto accountID = factory_.AccountIDFromBase58(account);

            map.emplace(accountID);
            std::get<5>(get_account_data(lock, accountID)) = type;
        }
    }
}

auto Accounts::Load(
    const UnallocatedCString& id,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    const bool checking) const -> bool
{
    return load_raw(id, output, alias, checking);
}

auto Accounts::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();
        OT_FAIL;
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Accounts::serialize() const -> proto::StorageAccounts
{
    proto::StorageAccounts serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_,
                item.first,
                item.second,
                *serialized.add_account(),
                proto::STORAGEHASH_RAW);
        }
    }

    SERIALIZE_INDEX(owner_index_, owner);
    SERIALIZE_INDEX(signer_index_, signer);
    SERIALIZE_INDEX(issuer_index_, issuer);
    SERIALIZE_INDEX(server_index_, server);
    SERIALIZE_INDEX(contract_index_, unit);

    for (const auto& [type, accounts] : unit_index_) {
        auto& listProto = *serialized.add_index();
        listProto.set_version(INDEX_VERSION);
        listProto.set_type(translate(UnitToClaim(type)));

        for (const auto& accountID : accounts) {
            if (accountID.empty()) { continue; }

            listProto.add_account(accountID.asBase58(crypto_));
        }

        if (0 == listProto.account_size()) {
            serialized.mutable_index()->RemoveLast();
        }
    }

    return serialized;
}

auto Accounts::SetAlias(
    const UnallocatedCString& id,
    const UnallocatedCString& alias) -> bool
{
    return set_alias(id, alias);
}

auto Accounts::Store(
    const UnallocatedCString& id,
    const UnallocatedCString& data,
    const UnallocatedCString& alias,
    const identifier::Nym& owner,
    const identifier::Nym& signer,
    const identifier::Nym& issuer,
    const identifier::Notary& server,
    const identifier::UnitDefinition& contract,
    const UnitType unit) -> bool
{
    Lock lock(write_lock_);
    const auto account = factory_.AccountIDFromBase58(id);

    if (!check_update_account(
            lock, account, owner, signer, issuer, server, contract, unit)) {

        return false;
    }

    return store_raw(lock, data, id, alias);
}
}  // namespace opentxs::storage

#undef INDEX_VERSION
#undef ACCOUNT_VERSION
#undef DESERIALIZE_INDEX
#undef SERIALIZE_INDEX
#undef EXTRACT_FIELD
#undef EXTRACT_SET_BY_ID
#undef EXTRACT_SET_BY_VALUE
