// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Accounts.hpp"

#include "opentxs/core/Log.hpp"

#include "storage/Plugin.hpp"

#define EXTRACT_SET_BY_VALUE(index, value)                                     \
    {                                                                          \
        try {                                                                  \
                                                                               \
            return index.at(value);                                            \
                                                                               \
        } catch (...) {                                                        \
                                                                               \
            return {};                                                         \
        }                                                                      \
    }

#define EXTRACT_SET_BY_ID(index, id)                                           \
    {                                                                          \
        EXTRACT_SET_BY_VALUE(index, Identifier::Factory(id))                   \
    }

#define EXTRACT_FIELD(field)                                                   \
    {                                                                          \
        Lock lock(write_lock_);                                                \
                                                                               \
        return std::get<field>(                                                \
            get_account_data(lock, Identifier::Factory(id)));                  \
    }

#define SERIALIZE_INDEX(index, field)                                          \
    {                                                                          \
        for (const auto& [id, accounts] : index) {                             \
            if (id->empty()) { continue; }                                     \
                                                                               \
            auto& listProto = *serialized.add_##field();                       \
            listProto.set_version(INDEX_VERSION);                              \
            listProto.set_id(id->str());                                       \
                                                                               \
            for (const auto& accountID : accounts) {                           \
                if (accountID->empty()) { continue; }                          \
                                                                               \
                listProto.add_list(accountID->str());                          \
            }                                                                  \
                                                                               \
            if (0 == listProto.list_size()) {                                  \
                serialized.mutable_##field()->RemoveLast();                    \
            }                                                                  \
        }                                                                      \
    }

#define DESERIALIZE_INDEX(field, index, position)                              \
    {                                                                          \
        for (const auto& it : serialized->field()) {                           \
            const auto id = Identifier::Factory(it.id());                      \
                                                                               \
            auto& map = index[id];                                             \
                                                                               \
            for (const auto& account : it.list()) {                            \
                const auto accountID = Identifier::Factory(account);           \
                                                                               \
                map.emplace(accountID);                                        \
                std::get<position>(get_account_data(lock, accountID))          \
                    ->SetString(id->str());                                    \
            }                                                                  \
        }                                                                      \
    }

#define ACCOUNT_VERSION 1
#define INDEX_VERSION 1

//#define OT_METHOD "opentxs::storage::Accounts::"

namespace opentxs::storage
{
Accounts::Accounts(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = ACCOUNT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

OTIdentifier Accounts::AccountContract(const Identifier& id) const
{
    EXTRACT_FIELD(4);
}

OTIdentifier Accounts::AccountIssuer(const Identifier& id) const
{
    EXTRACT_FIELD(2);
}

OTIdentifier Accounts::AccountOwner(const Identifier& id) const
{
    EXTRACT_FIELD(0);
}

OTIdentifier Accounts::AccountServer(const Identifier& id) const
{
    EXTRACT_FIELD(3);
}

OTIdentifier Accounts::AccountSigner(const Identifier& id) const
{
    EXTRACT_FIELD(1);
}

proto::ContactItemType Accounts::AccountUnit(const Identifier& id) const
{
    EXTRACT_FIELD(5);
}

std::set<OTIdentifier> Accounts::AccountsByContract(
    const Identifier& contract) const
{
    EXTRACT_SET_BY_ID(contract_index_, contract);
}

std::set<OTIdentifier> Accounts::AccountsByIssuer(
    const Identifier& issuerNym) const
{
    EXTRACT_SET_BY_ID(issuer_index_, issuerNym);
}

std::set<OTIdentifier> Accounts::AccountsByOwner(
    const Identifier& ownerNym) const
{
    EXTRACT_SET_BY_ID(owner_index_, ownerNym);
}

std::set<OTIdentifier> Accounts::AccountsByServer(
    const Identifier& server) const
{
    EXTRACT_SET_BY_ID(server_index_, server);
}

std::set<OTIdentifier> Accounts::AccountsByUnit(
    const proto::ContactItemType unit) const
{
    EXTRACT_SET_BY_VALUE(unit_index_, unit);
}

bool Accounts::add_set_index(
    const Identifier& accountID,
    const Identifier& argID,
    Identifier& mapID,
    Index& index)
{
    if (mapID.empty()) {
        index[argID].emplace(accountID);
        mapID.SetString(argID.str());
    } else {
        if (mapID != argID) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Provided index id (")(argID)(
                ") for account ")(accountID)(
                " does not match existing index id ")(mapID)(".")
                .Flush();

            return false;
        }

        OT_ASSERT(1 == index.at(argID).count(accountID))
    }

    return true;
}

std::string Accounts::Alias(const std::string& id) const
{
    return get_alias(id);
}

bool Accounts::check_update_account(
    const Lock& lock,
    const OTIdentifier& accountID,
    const Identifier& ownerNym,
    const Identifier& signerNym,
    const Identifier& issuerNym,
    const Identifier& server,
    const Identifier& contract,
    const proto::ContactItemType unit)
{
    if (accountID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid account ID.").Flush();

        return false;
    }

    if (ownerNym.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid owner nym ID.").Flush();

        return false;
    }

    if (signerNym.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signer nym ID.").Flush();

        return false;
    }

    if (issuerNym.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid issuer nym ID.").Flush();

        return false;
    }

    if (server.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid server ID.").Flush();

        return false;
    }

    if (contract.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid unit ID.").Flush();

        return false;
    }

    OT_ASSERT(verify_write_lock(lock))

    auto& [mapOwner, mapSigner, mapIssuer, mapServer, mapContract, mapUnit] =
        get_account_data(lock, accountID);

    if (!add_set_index(accountID, ownerNym, mapOwner, owner_index_)) {

        return false;
    }

    if (!add_set_index(accountID, signerNym, mapSigner, signer_index_)) {

        return false;
    }

    if (!add_set_index(accountID, issuerNym, mapIssuer, issuer_index_)) {

        return false;
    }

    if (!add_set_index(accountID, server, mapServer, server_index_)) {

        return false;
    }

    if (!add_set_index(accountID, contract, mapContract, contract_index_)) {
        return false;
    }

    if (proto::CITEMTYPE_UNKNOWN != unit) {
        mapUnit = unit;
        unit_index_[unit].emplace(accountID);
    }

    return true;
}

bool Accounts::Delete(const std::string& id)
{
    Lock lock(write_lock_);
    const auto accountID = Identifier::Factory(id);
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

Accounts::AccountData& Accounts::get_account_data(
    const Lock& lock,
    const OTIdentifier& accountID) const
{
    OT_ASSERT(verify_write_lock(lock))

    auto data = account_data_.find(accountID);

    if (account_data_.end() == data) {
        AccountData blank{Identifier::Factory(),
                          Identifier::Factory(),
                          Identifier::Factory(),
                          Identifier::Factory(),
                          Identifier::Factory(),
                          proto::CITEMTYPE_UNKNOWN};
        auto [output, added] =
            account_data_.emplace(accountID, std::move(blank));

        OT_ASSERT(added)

        return output->second;
    }

    return data->second;
}

void Accounts::init(const std::string& hash)
{
    Lock lock(write_lock_);
    std::shared_ptr<proto::StorageAccounts> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load account index file.")
            .Flush();
        OT_FAIL;
    }

    version_ = serialized->version();

    // Upgrade version
    if (ACCOUNT_VERSION > version_) { version_ = ACCOUNT_VERSION; }

    for (const auto& it : serialized->account()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    DESERIALIZE_INDEX(owner, owner_index_, 0)
    DESERIALIZE_INDEX(signer, signer_index_, 1)
    DESERIALIZE_INDEX(issuer, issuer_index_, 2)
    DESERIALIZE_INDEX(server, server_index_, 3)
    DESERIALIZE_INDEX(unit, contract_index_, 4)

    for (const auto& it : serialized->index()) {
        const auto unit = it.type();
        auto& map = unit_index_[unit];

        for (const auto& account : it.account()) {
            const auto accountID = Identifier::Factory(account);

            map.emplace(accountID);
            std::get<5>(get_account_data(lock, accountID)) = unit;
        }
    }
}

bool Accounts::Load(
    const std::string& id,
    std::string& output,
    std::string& alias,
    const bool checking) const
{
    return load_raw(id, output, alias, checking);
}

bool Accounts::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL;
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageAccounts Accounts::serialize() const
{
    proto::StorageAccounts serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                item.first,
                item.second,
                *serialized.add_account(),
                proto::STORAGEHASH_RAW);
        }
    }

    SERIALIZE_INDEX(owner_index_, owner)
    SERIALIZE_INDEX(signer_index_, signer)
    SERIALIZE_INDEX(issuer_index_, issuer)
    SERIALIZE_INDEX(server_index_, server)
    SERIALIZE_INDEX(contract_index_, unit)

    for (const auto& [type, accounts] : unit_index_) {
        auto& listProto = *serialized.add_index();
        listProto.set_version(INDEX_VERSION);
        listProto.set_type(type);

        for (const auto& accountID : accounts) {
            if (accountID->empty()) { continue; }

            listProto.add_account(accountID->str());
        }

        if (0 == listProto.account_size()) {
            serialized.mutable_index()->RemoveLast();
        }
    }

    return serialized;
}

bool Accounts::SetAlias(const std::string& id, const std::string& alias)
{
    return set_alias(id, alias);
}

bool Accounts::Store(
    const std::string& id,
    const std::string& data,
    const std::string& alias,
    const Identifier& owner,
    const Identifier& signer,
    const Identifier& issuer,
    const Identifier& server,
    const Identifier& contract,
    const proto::ContactItemType unit)
{
    Lock lock(write_lock_);
    const auto account = Identifier::Factory(id);

    if (!check_update_account(
            lock, account, owner, signer, issuer, server, contract, unit)) {

        return false;
    }

    return store_raw(lock, data, id, alias);
}
}  // namespace opentxs::storage
