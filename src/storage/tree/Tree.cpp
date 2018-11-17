// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Tree.hpp"

#include "storage/Plugin.hpp"
#include "Accounts.hpp"
#include "BlockchainTransactions.hpp"
#include "Contacts.hpp"
#include "Credentials.hpp"
#include "Nym.hpp"
#include "Nyms.hpp"
#include "Seeds.hpp"
#include "Servers.hpp"
#include "Units.hpp"

#define TREE_VERSION 4

//#define OT_METHOD "opentxs::storage::Tree::"

namespace opentxs::storage
{
Tree::Tree(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , account_root_(Node::BLANK_HASH)
    , blockchain_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , account_lock_()
    , account_(nullptr)
    , blockchain_lock_()
    , blockchain_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , nym_lock_()
    , nyms_(nullptr)
    , seed_lock_()
    , seeds_(nullptr)
    , server_lock_()
    , servers_(nullptr)
    , unit_lock_()
    , units_(nullptr)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = TREE_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

Tree::Tree(const Tree& rhs)
    : Node(rhs.driver_, "")
    , account_root_(Node::BLANK_HASH)
    , blockchain_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , account_lock_()
    , account_(nullptr)
    , blockchain_lock_()
    , blockchain_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , nym_lock_()
    , nyms_(nullptr)
    , seed_lock_()
    , seeds_(nullptr)
    , server_lock_()
    , servers_(nullptr)
    , unit_lock_()
    , units_(nullptr)
{
    Lock lock(rhs.write_lock_);

    version_ = rhs.version_;
    root_ = rhs.root_;
    account_root_ = rhs.account_root_;
    blockchain_root_ = rhs.blockchain_root_;
    contact_root_ = rhs.contact_root_;
    credential_root_ = rhs.credential_root_;
    nym_root_ = rhs.nym_root_;
    seed_root_ = rhs.seed_root_;
    server_root_ = rhs.server_root_;
    unit_root_ = rhs.unit_root_;
}

Accounts* Tree::accounts() const
{
    Lock lock(account_lock_);

    if (false == bool(account_)) {
        account_.reset(new Accounts(driver_, account_root_));

        if (false == bool(account_)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();

            OT_FAIL;
        }
    }

    lock.unlock();

    return account_.get();
}

BlockchainTransactions* Tree::blockchain() const
{
    Lock lock(blockchain_lock_);

    if (false == bool(blockchain_)) {
        blockchain_.reset(
            new BlockchainTransactions(driver_, blockchain_root_));

        if (false == bool(blockchain_)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();

            OT_FAIL
        }
    }

    lock.unlock();

    return blockchain_.get();
}

const Accounts& Tree::AccountNode() const { return *accounts(); }

const BlockchainTransactions& Tree::BlockchainNode() const
{
    return *blockchain();
}

const Contacts& Tree::ContactNode() const { return *contacts(); }

Contacts* Tree::contacts() const
{
    Lock lock(contact_lock_);

    if (!contacts_) {
        contacts_.reset(new Contacts(driver_, contact_root_));

        if (!contacts_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();

            OT_FAIL
        }
    }

    lock.unlock();

    return contacts_.get();
}

const Credentials& Tree::CredentialNode() const { return *credentials(); }

Credentials* Tree::credentials() const
{
    Lock lock(credential_lock_);

    if (!credentials_) {
        credentials_.reset(new Credentials(driver_, credential_root_));

        if (!credentials_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();

            OT_FAIL
        }
    }

    lock.unlock();

    return credentials_.get();
}

void Tree::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageItems> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load root index file.")
            .Flush();

        OT_FAIL
    }

    version_ = serialized->version();

    // Upgrade version
    if (TREE_VERSION > version_) { version_ = TREE_VERSION; }

    account_root_ = normalize_hash(serialized->accounts());
    blockchain_root_ = normalize_hash(serialized->blockchaintransactions());
    contact_root_ = normalize_hash(serialized->contacts());
    credential_root_ = normalize_hash(serialized->creds());
    nym_root_ = normalize_hash(serialized->nyms());
    seed_root_ = normalize_hash(serialized->seeds());
    server_root_ = normalize_hash(serialized->servers());
    unit_root_ = normalize_hash(serialized->units());
}

bool Tree::Migrate(const opentxs::api::storage::Driver& to) const
{
    bool output{true};
    output &= accounts()->Migrate(to);
    output &= blockchain()->Migrate(to);
    output &= contacts()->Migrate(to);
    output &= credentials()->Migrate(to);
    output &= nyms()->Migrate(to);
    output &= seeds()->Migrate(to);
    output &= servers()->Migrate(to);
    output &= units()->Migrate(to);
    output &= migrate(root_, to);

    return output;
}

Editor<Accounts> Tree::mutable_Accounts()
{
    std::function<void(Accounts*, Lock&)> callback =
        [&](Accounts* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Accounts>(write_lock_, accounts(), callback);
}

Editor<BlockchainTransactions> Tree::mutable_Blockchain()
{
    std::function<void(BlockchainTransactions*, Lock&)> callback =
        [&](BlockchainTransactions* in, Lock& lock) -> void {
        this->save(in, lock);
    };

    return Editor<BlockchainTransactions>(write_lock_, blockchain(), callback);
}

Editor<Contacts> Tree::mutable_Contacts()
{
    std::function<void(Contacts*, Lock&)> callback =
        [&](Contacts* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Contacts>(write_lock_, contacts(), callback);
}

Editor<Credentials> Tree::mutable_Credentials()
{
    std::function<void(Credentials*, Lock&)> callback =
        [&](Credentials* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Credentials>(write_lock_, credentials(), callback);
}

Editor<Nyms> Tree::mutable_Nyms()
{
    std::function<void(Nyms*, Lock&)> callback =
        [&](Nyms* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Nyms>(write_lock_, nyms(), callback);
}

Editor<Seeds> Tree::mutable_Seeds()
{
    std::function<void(Seeds*, Lock&)> callback =
        [&](Seeds* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Seeds>(write_lock_, seeds(), callback);
}

Editor<Servers> Tree::mutable_Servers()
{
    std::function<void(Servers*, Lock&)> callback =
        [&](Servers* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Servers>(write_lock_, servers(), callback);
}

Editor<Units> Tree::mutable_Units()
{
    std::function<void(Units*, Lock&)> callback =
        [&](Units* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<Units>(write_lock_, units(), callback);
}

const Nyms& Tree::NymNode() const { return *nyms(); }

Nyms* Tree::nyms() const
{
    Lock lock(nym_lock_);

    if (!nyms_) {
        nyms_.reset(new Nyms(driver_, nym_root_));

        if (!nyms_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();
            OT_FAIL
        }
    }

    lock.unlock();

    return nyms_.get();
}

bool Tree::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

void Tree::save(Accounts* accounts, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == account_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(account_lock_);
    account_root_ = account_->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

void Tree::save(BlockchainTransactions* blockchain, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == blockchain) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(blockchain_lock_);
    blockchain_root_ = blockchain->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

void Tree::save(Contacts* contacts, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == contacts) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(contact_lock_);
    contact_root_ = contacts->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

void Tree::save(Credentials* credentials, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == credentials) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(credential_lock_);
    credential_root_ = credentials->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

void Tree::save(Nyms* nyms, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == nyms) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(nym_lock_);
    nym_root_ = nyms->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

void Tree::save(Seeds* seeds, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == seeds) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(seed_lock_);
    seed_root_ = seeds->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

void Tree::save(Servers* servers, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == servers) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(server_lock_);
    server_root_ = servers->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

void Tree::save(Units* units, const Lock& lock)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == units) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock mapLock(unit_lock_);
    unit_root_ = units->Root();
    mapLock.unlock();

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        OT_FAIL
    }
}

const Seeds& Tree::SeedNode() const { return *seeds(); }

Seeds* Tree::seeds() const
{
    Lock lock(seed_lock_);

    if (!seeds_) {
        seeds_.reset(new Seeds(driver_, seed_root_));

        if (!seeds_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();
            OT_FAIL
        }
    }

    lock.unlock();

    return seeds_.get();
}

proto::StorageItems Tree::serialize() const
{
    proto::StorageItems serialized;
    serialized.set_version(version_);

    Lock accountLock(account_lock_);
    serialized.set_accounts(account_root_);
    accountLock.unlock();

    Lock blockchainLock(blockchain_lock_);
    serialized.set_blockchaintransactions(blockchain_root_);
    blockchainLock.unlock();

    Lock contactLock(contact_lock_);
    serialized.set_contacts(contact_root_);
    contactLock.unlock();

    Lock credLock(credential_lock_);
    serialized.set_creds(credential_root_);
    credLock.unlock();

    Lock nymLock(nym_lock_);
    serialized.set_nyms(nym_root_);
    nymLock.unlock();

    Lock seedLock(seed_lock_);
    serialized.set_seeds(seed_root_);
    seedLock.unlock();

    Lock serverLock(server_lock_);
    serialized.set_servers(server_root_);
    serverLock.unlock();

    Lock unitLock(unit_lock_);
    serialized.set_units(unit_root_);
    unitLock.unlock();

    return serialized;
}

const Servers& Tree::ServerNode() const { return *servers(); }

Servers* Tree::servers() const
{
    Lock lock(server_lock_);

    if (!servers_) {
        servers_.reset(new Servers(driver_, server_root_));

        if (!servers_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();
            OT_FAIL
        }
    }

    lock.unlock();

    return servers_.get();
}

const Units& Tree::UnitNode() const { return *units(); }

Units* Tree::units() const
{
    Lock lock(unit_lock_);

    if (!units_) {
        units_.reset(new Units(driver_, unit_root_));

        if (!units_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to instantiate.")
                .Flush();
            OT_FAIL
        }
    }

    lock.unlock();

    return units_.get();
}

Tree::~Tree() {}
}  // namespace opentxs::storage
