// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Accounts.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/StorageAccountIndex.pb.h>
#include <opentxs/protobuf/StorageAccounts.pb.h>
#include <opentxs/protobuf/StorageEnums.pb.h>
#include <opentxs/protobuf/StorageIDList.pb.h>
#include <atomic>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <utility>

#include "internal/util/DeferredConstruction.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/UnitType.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/StorageAccounts.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Log.hpp"
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
        auto lock = Lock{write_lock_};                                         \
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
    for (const auto& it : proto.field()) {                                     \
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

namespace opentxs::storage::tree
{
using namespace std::literals;

Accounts::Accounts(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash)
    : Node(
          crypto,
          factory,
          storage,
          hash,
          std::source_location::current().function_name(),
          ACCOUNT_VERSION)
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
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
            LogError()()("Provided index id (")(argID, crypto)(
                ") for account ")(accountID, crypto)(
                " does not match existing index id ")(mapID, crypto)
                .Flush();

            return false;
        }

        assert_true(1 == index.at(argID).count(accountID));
    }

    return true;
}

auto Accounts::Alias(const identifier::Account& id) const -> UnallocatedCString
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
        LogError()()("Invalid account ID.").Flush();

        return false;
    }

    if (ownerNym.empty()) {
        LogError()()("Invalid owner nym ID.").Flush();

        return false;
    }

    if (signerNym.empty()) {
        LogError()()("Invalid signer nym ID.").Flush();

        return false;
    }

    if (issuerNym.empty()) {
        LogError()()("Invalid issuer nym ID.").Flush();

        return false;
    }

    if (server.empty()) {
        LogError()()("Invalid server ID.").Flush();

        return false;
    }

    if (contract.empty()) {
        LogError()()("Invalid unit ID.").Flush();

        return false;
    }

    assert_true(verify_write_lock(lock));

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

auto Accounts::Delete(const identifier::Account& id) -> bool
{
    const auto lock = Lock{write_lock_};
    auto it = account_data_.find(id);

    if (account_data_.end() != it) {
        const auto& [owner, signer, issuer, server, contract, unit] =
            it->second;
        erase(id, owner, owner_index_);
        erase(id, signer, signer_index_);
        erase(id, issuer, issuer_index_);
        erase(id, server, server_index_);
        erase(id, contract, contract_index_);
        erase(id, unit, unit_index_);
        account_data_.erase(it);
    }

    return delete_item(lock, id);
}

auto Accounts::get_account_data(
    const Lock& lock,
    const identifier::Account& accountID) const -> Accounts::AccountData&
{
    assert_true(verify_write_lock(lock));

    auto data = account_data_.find(accountID);

    if (account_data_.end() == data) {
        auto [output, added] = account_data_.emplace(
            accountID, AccountData{{}, {}, {}, {}, {}, UnitType::Unknown});

        assert_true(added);

        return output->second;
    }

    return data->second;
}

auto Accounts::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<protobuf::StorageAccounts>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 1u:
            default: {
                init_map(proto.account());

                const auto lock = Lock{write_lock_};
                DESERIALIZE_INDEX(owner, owner_index_, 0, NymIDFromBase58);
                DESERIALIZE_INDEX(signer, signer_index_, 1, NymIDFromBase58);
                DESERIALIZE_INDEX(issuer, issuer_index_, 2, NymIDFromBase58);
                DESERIALIZE_INDEX(server, server_index_, 3, NotaryIDFromBase58);
                DESERIALIZE_INDEX(unit, contract_index_, 4, UnitIDFromBase58);

                for (const auto& it : proto.index()) {
                    const auto unit = it.type();
                    const auto type = ClaimToUnit(translate(unit));
                    auto& map = unit_index_[type];

                    for (const auto& account : it.account()) {
                        const auto accountID =
                            factory_.AccountIDFromBase58(account);

                        map.emplace(accountID);
                        std::get<5>(get_account_data(lock, accountID)) = type;
                    }
                }
            }
        }
    } else {
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto Accounts::Load(
    const identifier::Account& id,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const -> bool
{
    return load_raw(id, output, alias, checking);
}

auto Accounts::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogError()()("Lock failure.").Flush();
        LogAbort()().Abort();
    }

    auto serialized = serialize();

    if (false == protobuf::syntax::check(LogError(), serialized)) {
        return false;
    }

    return StoreProto(serialized, root_);
}

auto Accounts::serialize() const -> protobuf::StorageAccounts
{
    protobuf::StorageAccounts serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                item.first,
                item.second,
                *serialized.add_account(),
                protobuf::STORAGEHASH_RAW);
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

auto Accounts::SetAlias(const identifier::Account& id, std::string_view alias)
    -> bool
{
    return set_alias(id, alias);
}

auto Accounts::Store(
    const identifier::Account& id,
    const UnallocatedCString& data,
    std::string_view alias,
    const identifier::Nym& owner,
    const identifier::Nym& signer,
    const identifier::Nym& issuer,
    const identifier::Notary& server,
    const identifier::UnitDefinition& contract,
    const UnitType unit) -> bool
{
    const auto lock = Lock{write_lock_};

    if (!check_update_account(
            lock, id, owner, signer, issuer, server, contract, unit)) {

        return false;
    }

    return store_raw(lock, data, id, alias);
}

auto Accounts::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        default: {
        }
    }

    return changed;
}
}  // namespace opentxs::storage::tree

#undef INDEX_VERSION
#undef ACCOUNT_VERSION
#undef DESERIALIZE_INDEX
#undef SERIALIZE_INDEX
#undef EXTRACT_FIELD
#undef EXTRACT_SET_BY_ID
#undef EXTRACT_SET_BY_VALUE
