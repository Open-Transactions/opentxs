// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Trunk.hpp"  // IWYU pragma: associated

#include <Ciphertext.pb.h>
#include <StorageItems.pb.h>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <string_view>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageItems.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Account.hpp"         // IWYU pragma: keep
#include "opentxs/core/identifier/Notary.hpp"          // IWYU pragma: keep
#include "opentxs/core/identifier/UnitDefinition.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Accounts.hpp"
#include "util/storage/tree/Contacts.hpp"
#include "util/storage/tree/Credentials.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Notary.hpp"
#include "util/storage/tree/Nym.hpp"  // IWYU pragma: keep
#include "util/storage/tree/Nyms.hpp"
#include "util/storage/tree/Seeds.hpp"
#include "util/storage/tree/Servers.hpp"
#include "util/storage/tree/Units.hpp"

#define TREE_VERSION 6

namespace opentxs::storage::tree
{
using namespace std::literals;

Trunk::Trunk(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash)
    : Node(crypto, factory, storage, hash, OT_PRETTY_CLASS(), TREE_VERSION)
    , account_root_(NullHash{})
    , contact_root_(NullHash{})
    , credential_root_(NullHash{})
    , notary_root_(NullHash{})
    , nym_root_(NullHash{})
    , seed_root_(NullHash{})
    , server_root_(NullHash{})
    , unit_root_(NullHash{})
    , account_lock_()
    , account_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , notary_lock_()
    , notary_(nullptr)
    , nym_lock_()
    , nyms_(nullptr)
    , seed_lock_()
    , seeds_(nullptr)
    , server_lock_()
    , servers_(nullptr)
    , unit_lock_()
    , units_(nullptr)
    , master_key_lock_()
    , master_key_(nullptr)
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Trunk::accounts() const -> tree::Accounts*
{
    return get_child<tree::Accounts>(account_lock_, account_, account_root_);
}

auto Trunk::Accounts() const -> const tree::Accounts& { return *accounts(); }

auto Trunk::Contacts() const -> const tree::Contacts& { return *contacts(); }

auto Trunk::contacts() const -> tree::Contacts*
{
    return get_child<tree::Contacts>(contact_lock_, contacts_, contact_root_);
}

auto Trunk::Credentials() const -> const tree::Credentials&
{
    return *credentials();
}

auto Trunk::credentials() const -> tree::Credentials*
{
    return get_child<tree::Credentials>(
        credential_lock_, credentials_, credential_root_);
}

auto Trunk::dump(const Lock& lock, const Log& log, Vector<Hash>& out)
    const noexcept -> bool
{
    if (false == is_valid(root_)) { return true; }

    if (false == Node::dump(lock, log, out)) { return false; }

    if (is_valid(account_root_)) {
        if (false == accounts()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(contact_root_)) {
        if (false == contacts()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(credential_root_)) {
        if (false == credentials()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(notary_root_)) {
        if (false == notary({})->dump(lock, log, out)) { return false; }
    }

    if (is_valid(nym_root_)) {
        if (false == nyms()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(seed_root_)) {
        if (false == seeds()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(server_root_)) {
        if (false == servers()->dump(lock, log, out)) { return false; }
    }

    if (is_valid(unit_root_)) {
        if (false == units()->dump(lock, log, out)) { return false; }
    }

    return true;
}

template <typename T, typename... Args>
auto Trunk::get_child(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    const Hash& hash,
    Args&&... params) const -> T*
{
    Lock lock(mutex);

    if (false == bool(pointer)) {
        pointer.reset(new T(crypto_, factory_, plugin_, hash, params...));

        if (false == bool(pointer)) {
            LogAbort()(OT_PRETTY_CLASS())("Unable to instantiate.").Abort();
        }
    }

    lock.unlock();

    return pointer.get();
}

template <typename T, typename... Args>
auto Trunk::get_editor(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    Hash& hash,
    Args&&... params) const -> Editor<T>
{
    std::function<void(T*, Lock&)> callback = [&](T* in, Lock& lock) -> void {
        save_child<T>(in, lock, mutex, hash);
    };

    return Editor<T>(
        write_lock_, get_child<T>(mutex, pointer, hash, params...), callback);
}

auto Trunk::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageItems>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 6u:
            case 5u:
            case 4u:
            case 3u:
            case 2u:
            case 1u:
            default: {
                account_root_ = read(proto.accounts());
                // NOTE blockchaintransactions field is no longer used
                contact_root_ = read(proto.contacts());
                credential_root_ = read(proto.creds());
                notary_root_ = read(proto.notary());
                nym_root_ = read(proto.nyms());
                seed_root_ = read(proto.seeds());
                server_root_ = read(proto.servers());
                unit_root_ = read(proto.units());

                if (proto.has_master_secret()) {
                    master_key_ = std::make_shared<proto::Ciphertext>(
                        proto.master_secret());
                }
            }
        }
    } else {
        throw std::runtime_error{
            "failed to load root object file in "s.append(OT_PRETTY_CLASS())};
    }
}

auto Trunk::Load(
    std::shared_ptr<proto::Ciphertext>& output,
    ErrorReporting checking) const -> bool
{
    Lock lock(master_key_lock_);

    const bool have = bool(master_key_);

    if (have) {
        output = master_key_;

        return true;
    } else {
        using enum ErrorReporting;

        if (verbose == checking) {
            LogError()(OT_PRETTY_CLASS())("Master key does not exist.").Flush();
        }
    }

    return false;
}

auto Trunk::mutable_Accounts() -> Editor<tree::Accounts>
{
    return get_editor<tree::Accounts>(account_lock_, account_, account_root_);
}

auto Trunk::mutable_Contacts() -> Editor<tree::Contacts>
{
    return get_editor<tree::Contacts>(contact_lock_, contacts_, contact_root_);
}

auto Trunk::mutable_Credentials() -> Editor<tree::Credentials>
{
    return get_editor<tree::Credentials>(
        credential_lock_, credentials_, credential_root_);
}

auto Trunk::mutable_Notary(const identifier::Notary& id) -> Editor<tree::Notary>
{
    return get_editor<tree::Notary>(notary_lock_, notary_, notary_root_, id);
}

auto Trunk::mutable_Nyms() -> Editor<tree::Nyms>
{
    return get_editor<tree::Nyms>(nym_lock_, nyms_, nym_root_);
}

auto Trunk::mutable_Seeds() -> Editor<tree::Seeds>
{
    return get_editor<tree::Seeds>(seed_lock_, seeds_, seed_root_);
}

auto Trunk::mutable_Servers() -> Editor<tree::Servers>
{
    return get_editor<tree::Servers>(server_lock_, servers_, server_root_);
}

auto Trunk::mutable_Units() -> Editor<tree::Units>
{
    return get_editor<tree::Units>(unit_lock_, units_, unit_root_);
}

auto Trunk::Notary(const identifier::Notary& id) const -> const tree::Notary&
{
    return *notary(id);
}

auto Trunk::Nyms() const -> const tree::Nyms& { return *nyms(); }

auto Trunk::notary(const identifier::Notary& id) const -> tree::Notary*
{
    return get_child<tree::Notary>(notary_lock_, notary_, notary_root_, id);
}

auto Trunk::nyms() const -> tree::Nyms*
{
    return get_child<tree::Nyms>(nym_lock_, nyms_, nym_root_);
}

auto Trunk::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogAbort()(OT_PRETTY_CLASS())("Lock failure.").Abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return StoreProto(serialized, root_);
}

template <typename T>
void Trunk::save_child(
    T* input,
    const Lock& lock,
    std::mutex& hashLock,
    Hash& hash) const
{
    if (false == verify_write_lock(lock)) {
        LogAbort()(OT_PRETTY_CLASS())("Lock failure.").Abort();
    }

    if (nullptr == input) {
        LogAbort()(OT_PRETTY_CLASS())("Null target.").Abort();
    }

    Lock rootLock(hashLock);
    hash = input->Root();
    rootLock.unlock();

    if (false == save(lock)) {
        LogAbort()(OT_PRETTY_CLASS())("Save error.").Abort();
    }
}

auto Trunk::Seeds() const -> const tree::Seeds& { return *seeds(); }

auto Trunk::seeds() const -> tree::Seeds*
{
    return get_child<tree::Seeds>(seed_lock_, seeds_, seed_root_);
}

auto Trunk::serialize() const -> proto::StorageItems
{
    auto proto = proto::StorageItems{};
    proto.set_version(version_);

    {
        Lock accountLock(account_lock_);
        write(account_root_, *proto.mutable_accounts());
    }
    {
        Lock contactLock(contact_lock_);
        write(contact_root_, *proto.mutable_contacts());
    }
    {
        Lock credLock(credential_lock_);
        write(credential_root_, *proto.mutable_creds());
    }
    {
        Lock notaryLock(notary_lock_);
        write(notary_root_, *proto.mutable_notary());
    }
    {
        Lock nymLock(nym_lock_);
        write(nym_root_, *proto.mutable_nyms());
    }
    {
        Lock seedLock(seed_lock_);
        write(seed_root_, *proto.mutable_seeds());
    }
    {
        Lock serverLock(server_lock_);
        write(server_root_, *proto.mutable_servers());
    }
    {
        Lock unitLock(unit_lock_);
        write(unit_root_, *proto.mutable_units());
    }
    {
        Lock masterLock(master_key_lock_);

        if (master_key_) { *proto.mutable_master_secret() = *master_key_; }
    }

    return proto;
}

auto Trunk::Servers() const -> const tree::Servers& { return *servers(); }

auto Trunk::servers() const -> tree::Servers*
{
    return get_child<tree::Servers>(server_lock_, servers_, server_root_);
}

auto Trunk::Store(const proto::Ciphertext& serialized) -> bool
{
    Lock masterLock(master_key_lock_, std::defer_lock);
    Lock writeLock(write_lock_, std::defer_lock);
    std::lock(masterLock, writeLock);
    master_key_ = std::make_shared<proto::Ciphertext>(serialized);
    masterLock.unlock();

    return save(writeLock);
}

auto Trunk::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        case 2u:
        case 3u:
        case 4u:
        case 5u:
        case 6u:
        default: {
        }
    }

    if (is_valid(account_root_)) {
        if (auto* node = accounts(); node->Upgrade()) {
            account_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(contact_root_)) {
        if (auto* node = contacts(); node->Upgrade()) {
            contact_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(credential_root_)) {
        if (auto* node = credentials(); node->Upgrade()) {
            credential_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(notary_root_)) {
        if (auto* node = notary({}); node->Upgrade()) {
            notary_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(nym_root_)) {
        if (auto* node = nyms(); node->Upgrade()) {
            nym_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(seed_root_)) {
        if (auto* node = seeds(); node->Upgrade()) {
            seed_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(server_root_)) {
        if (auto* node = servers(); node->Upgrade()) {
            server_root_ = node->root_;
            changed = true;
        }
    }

    if (is_valid(unit_root_)) {
        if (auto* node = units(); node->Upgrade()) {
            unit_root_ = node->root_;
            changed = true;
        }
    }

    return changed;
}

auto Trunk::Units() const -> const tree::Units& { return *units(); }

auto Trunk::units() const -> tree::Units*
{
    return get_child<tree::Units>(unit_lock_, units_, unit_root_);
}

Trunk::~Trunk() = default;
}  // namespace opentxs::storage::tree
