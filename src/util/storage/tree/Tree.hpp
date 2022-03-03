// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/StorageItems.pb.h"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
}  // namespace api

namespace proto
{
class Ciphertext;
}  // namespace proto

namespace storage
{
class Accounts;
class Contacts;
class Credentials;
class Driver;
class Notary;
class Nyms;
class Root;
class Seeds;
class Servers;
class Units;
}  // namespace storage
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Tree final : public Node
{
public:
    auto Accounts() const -> const storage::Accounts&;
    auto Contacts() const -> const storage::Contacts&;
    auto Credentials() const -> const storage::Credentials&;
    auto Notary(const UnallocatedCString& id) const -> const storage::Notary&;
    auto Nyms() const -> const storage::Nyms&;
    auto Seeds() const -> const storage::Seeds&;
    auto Servers() const -> const storage::Servers&;
    auto Units() const -> const storage::Units&;

    auto mutable_Accounts() -> Editor<storage::Accounts>;
    auto mutable_Contacts() -> Editor<storage::Contacts>;
    auto mutable_Credentials() -> Editor<storage::Credentials>;
    auto mutable_Notary(const UnallocatedCString& id)
        -> Editor<storage::Notary>;
    auto mutable_Nyms() -> Editor<storage::Nyms>;
    auto mutable_Seeds() -> Editor<storage::Seeds>;
    auto mutable_Servers() -> Editor<storage::Servers>;
    auto mutable_Units() -> Editor<storage::Units>;

    auto Load(
        std::shared_ptr<proto::Ciphertext>& output,
        const bool checking = false) const -> bool;
    auto Migrate(const Driver& to) const -> bool final;

    auto Store(const proto::Ciphertext& serialized) -> bool;

    ~Tree() final;

private:
    friend api::imp::Storage;
    friend storage::Root;

    const api::session::Factory& factory_;

    UnallocatedCString account_root_{Node::BLANK_HASH};
    UnallocatedCString contact_root_{Node::BLANK_HASH};
    UnallocatedCString credential_root_{Node::BLANK_HASH};
    UnallocatedCString notary_root_{Node::BLANK_HASH};
    UnallocatedCString nym_root_{Node::BLANK_HASH};
    UnallocatedCString seed_root_{Node::BLANK_HASH};
    UnallocatedCString server_root_{Node::BLANK_HASH};
    UnallocatedCString unit_root_{Node::BLANK_HASH};

    mutable std::mutex account_lock_;
    mutable std::unique_ptr<storage::Accounts> account_;
    mutable std::mutex contact_lock_;
    mutable std::unique_ptr<storage::Contacts> contacts_;
    mutable std::mutex credential_lock_;
    mutable std::unique_ptr<storage::Credentials> credentials_;
    mutable std::mutex notary_lock_;
    mutable std::unique_ptr<storage::Notary> notary_;
    mutable std::mutex nym_lock_;
    mutable std::unique_ptr<storage::Nyms> nyms_;
    mutable std::mutex seed_lock_;
    mutable std::unique_ptr<storage::Seeds> seeds_;
    mutable std::mutex server_lock_;
    mutable std::unique_ptr<storage::Servers> servers_;
    mutable std::mutex unit_lock_;
    mutable std::unique_ptr<storage::Units> units_;
    mutable std::mutex master_key_lock_;
    mutable std::shared_ptr<proto::Ciphertext> master_key_;

    template <typename T, typename... Args>
    auto get_child(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        const UnallocatedCString& hash,
        Args&&... params) const -> T*;
    template <typename T, typename... Args>
    auto get_editor(
        std::mutex& mutex,
        std::unique_ptr<T>& pointer,
        UnallocatedCString& hash,
        Args&&... params) const -> Editor<T>;
    auto accounts() const -> storage::Accounts*;
    auto contacts() const -> storage::Contacts*;
    auto credentials() const -> storage::Credentials*;
    auto notary(const UnallocatedCString& id) const -> storage::Notary*;
    auto nyms() const -> storage::Nyms*;
    auto seeds() const -> storage::Seeds*;
    auto servers() const -> storage::Servers*;
    auto units() const -> storage::Units*;

    void init(const UnallocatedCString& hash) final;
    auto save(const Lock& lock) const -> bool final;
    template <typename T>
    void save_child(
        T*,
        const Lock& lock,
        std::mutex& hashLock,
        UnallocatedCString& hash) const;
    auto serialize() const -> proto::StorageItems;
    auto update_root(const UnallocatedCString& hash) -> bool;

    Tree(
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& key);
    Tree() = delete;
    Tree(const Tree&);
    Tree(Tree&&) = delete;
    auto operator=(const Tree&) -> Tree = delete;
    auto operator=(Tree&&) -> Tree = delete;
};
}  // namespace opentxs::storage
