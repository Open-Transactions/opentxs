// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/StorageItems.pb.h>
#include <memory>
#include <mutex>

#include "internal/util/Editor.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace imp
{
class Storage;
}  // namespace imp

namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

namespace protobuf
{
class Ciphertext;
}  // namespace protobuf

namespace storage
{
namespace tree
{
class Accounts;
class Contacts;
class Credentials;
class GC;
class Notary;
class Nyms;
class Root;
class Seeds;
class Servers;
class Units;
}  // namespace tree

namespace driver
{
class Plugin;
}  // namespace driver
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Trunk final : public Node
{
public:
    auto Accounts() const -> const tree::Accounts&;
    auto Contacts() const -> const tree::Contacts&;
    auto Credentials() const -> const tree::Credentials&;
    auto Notary(const identifier::Notary& id) const -> const tree::Notary&;
    auto Nyms() const -> const tree::Nyms&;
    auto Seeds() const -> const tree::Seeds&;
    auto Servers() const -> const tree::Servers&;
    auto Units() const -> const tree::Units&;

    auto mutable_Accounts() -> Editor<tree::Accounts>;
    auto mutable_Contacts() -> Editor<tree::Contacts>;
    auto mutable_Credentials() -> Editor<tree::Credentials>;
    auto mutable_Notary(const identifier::Notary& id) -> Editor<tree::Notary>;
    auto mutable_Nyms() -> Editor<tree::Nyms>;
    auto mutable_Seeds() -> Editor<tree::Seeds>;
    auto mutable_Servers() -> Editor<tree::Servers>;
    auto mutable_Units() -> Editor<tree::Units>;

    auto Load(
        std::shared_ptr<protobuf::Ciphertext>& output,
        ErrorReporting checking = verbose) const -> bool;

    auto Store(const protobuf::Ciphertext& serialized) -> bool;

    Trunk() = delete;
    Trunk(const Trunk&) = delete;
    Trunk(Trunk&&) = delete;
    auto operator=(const Trunk&) -> Trunk = delete;
    auto operator=(Trunk&&) -> Trunk = delete;

    ~Trunk() final;

private:
    friend api::imp::Storage;
    friend tree::GC;
    friend tree::Root;

    Hash account_root_;
    Hash contact_root_;
    Hash credential_root_;
    Hash notary_root_;
    Hash nym_root_;
    Hash seed_root_;
    Hash server_root_;
    Hash unit_root_;

    mutable std::mutex account_lock_;
    mutable std::unique_ptr<tree::Accounts> account_;
    mutable std::mutex contact_lock_;
    mutable std::unique_ptr<tree::Contacts> contacts_;
    mutable std::mutex credential_lock_;
    mutable std::unique_ptr<tree::Credentials> credentials_;
    mutable std::mutex notary_lock_;
    mutable std::unique_ptr<tree::Notary> notary_;
    mutable std::mutex nym_lock_;
    mutable std::unique_ptr<tree::Nyms> nyms_;
    mutable std::mutex seed_lock_;
    mutable std::unique_ptr<tree::Seeds> seeds_;
    mutable std::mutex server_lock_;
    mutable std::unique_ptr<tree::Servers> servers_;
    mutable std::mutex unit_lock_;
    mutable std::unique_ptr<tree::Units> units_;
    mutable std::mutex master_key_lock_;
    mutable std::shared_ptr<protobuf::Ciphertext> master_key_;

    template <typename T, typename... Args>
    auto get_child(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        const Hash& hash,
        Args&&... params) const -> T*;
    template <typename T, typename... Args>
    auto get_editor(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        Hash& hash,
        Args&&... params) const -> Editor<T>;
    auto accounts() const -> tree::Accounts*;
    auto contacts() const -> tree::Contacts*;
    auto credentials() const -> tree::Credentials*;
    auto dump(const Lock&, const Log&, Vector<Hash>& out) const noexcept
        -> bool final;
    auto notary(const identifier::Notary& id) const -> tree::Notary*;
    auto nyms() const -> tree::Nyms*;
    auto seeds() const -> tree::Seeds*;
    auto servers() const -> tree::Servers*;
    auto units() const -> tree::Units*;

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const Lock& lock) const -> bool final;
    template <typename T>
    void save_child(T*, const Lock& lock, std::mutex& hashLock, Hash& hash)
        const;
    auto serialize() const -> protobuf::StorageItems;
    auto update_root(const Hash& hash) -> bool;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Trunk(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& key);
};
}  // namespace opentxs::storage::tree
