// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageEnums.pb.h>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace proto
{
class Contact;
class Nym;
class Seed;
class StorageItemHash;
}  // namespace proto

namespace storage
{
class Driver;
class Root;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
using keyFunction = std::function<bool(const UnallocatedCString&)>;
/** A set of metadata associated with a stored object
 *  * string: hash
 *  * string: alias
 *  * uint64: revision
 *  * bool:   private
 */
using Metadata =
    std::tuple<UnallocatedCString, UnallocatedCString, std::uint64_t, bool>;
/** Maps a logical id to the stored metadata for the object
 *  * string: id of the stored object
 *  * Metadata: metadata for the stored object
 */
using Index = UnallocatedMap<UnallocatedCString, Metadata>;

class Node
{
public:
    virtual auto List() const -> ObjectList;
    virtual auto Migrate(const Driver& to) const -> bool;
    auto Root() const -> UnallocatedCString;
    auto UpgradeLevel() const -> VersionNumber;

    Node() = delete;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    auto operator=(const Node&) -> Node& = delete;
    auto operator=(Node&&) -> Node& = delete;

    virtual ~Node() = default;

protected:
    template <class T>
    auto store_proto(
        const Lock& lock,
        const T& data,
        const UnallocatedCString& id,
        const UnallocatedCString& alias,
        UnallocatedCString& plaintext) -> bool
    {
        OT_ASSERT(verify_write_lock(lock));

        auto& metadata = item_map_[id];
        auto& hash = std::get<0>(metadata);

        if (!driver_.StoreProto<T>(data, hash, plaintext)) { return false; }

        if (!alias.empty()) { std::get<1>(metadata) = alias; }

        return save(lock);
    }

    template <class T>
    auto store_proto(
        const T& data,
        const UnallocatedCString& id,
        const UnallocatedCString& alias,
        UnallocatedCString& plaintext) -> bool
    {
        Lock lock(write_lock_);

        return store_proto(lock, data, id, alias, plaintext);
    }

    template <class T>
    auto store_proto(
        const T& data,
        const UnallocatedCString& id,
        const UnallocatedCString& alias) -> bool
    {
        UnallocatedCString notUsed;

        return store_proto<T>(data, id, alias, notUsed);
    }

    template <class T>
    auto load_proto(
        const UnallocatedCString& id,
        std::shared_ptr<T>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool
    {
        if (id.empty()) { return false; }

        Lock lock(write_lock_);
        const auto& it = item_map_.find(id);
        const bool exists = (item_map_.end() != it);

        if (!exists) {
            if (!checking) {
                std::cout << __func__ << ": Error: item with id " << id
                          << " does not exist." << std::endl;
            }

            return false;
        }

        alias = std::get<1>(it->second);

        return driver_.LoadProto<T>(std::get<0>(it->second), output, checking);
    }

    template <class T>
    void map(const std::function<void(const T&)> input) const
    {
        Lock lock(write_lock_);
        const auto copy = item_map_;
        lock.unlock();

        for (const auto& it : copy) {
            const auto& hash = std::get<0>(it.second);
            std::shared_ptr<T> serialized;

            if (Node::BLANK_HASH == hash) { continue; }

            if (driver_.LoadProto<T>(hash, serialized, false)) {
                input(*serialized);
            }
        }
    }

    template <class T>
    auto check_revision(
        const UnallocatedCString& method,
        const std::uint64_t incoming,
        Metadata& metadata) -> bool
    {
        const auto& hash = std::get<0>(metadata);
        auto& revision = std::get<2>(metadata);

        // This variable can be zero for two reasons:
        // * The stored version has never been incremented,
        // * The stored version hasn't been loaded yet and so the index
        // hasn't been updated
        // ...so we have to load the object just to be sure
        if (0 == revision) {
            std::shared_ptr<T> existing{nullptr};

            if (false == driver_.LoadProto(hash, existing, false)) {
                LogError()(method)(__func__)(": Unable to load object.")
                    .Flush();

                abort();
            }

            revision = extract_revision(*existing);
        }

        return (incoming > revision);
    }

protected:
    friend storage::Root;

    static const UnallocatedCString BLANK_HASH;

    const api::Crypto& crypto_;
    const api::session::Factory& factory_;
    const Driver& driver_;
    VersionNumber version_;
    VersionNumber original_version_;
    mutable UnallocatedCString root_;
    mutable std::mutex write_lock_;
    mutable Index item_map_;

    static auto normalize_hash(const UnallocatedCString& hash)
        -> UnallocatedCString;

    auto check_hash(const UnallocatedCString& hash) const -> bool;
    auto extract_revision(const proto::Contact& input) const -> std::uint64_t;
    auto extract_revision(const proto::Nym& input) const -> std::uint64_t;
    auto extract_revision(const proto::Seed& input) const -> std::uint64_t;
    auto get_alias(const UnallocatedCString& id) const -> UnallocatedCString;
    auto load_raw(
        const UnallocatedCString& id,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;
    auto migrate(const UnallocatedCString& hash, const Driver& to) const
        -> bool;
    virtual auto save(const Lock& lock) const -> bool = 0;
    void serialize_index(
        const VersionNumber version,
        const UnallocatedCString& id,
        const Metadata& metadata,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const;

    virtual void blank(const VersionNumber version);
    template <typename Serialized>
    void init_version(const VersionNumber version, const Serialized& serialized)
    {
        original_version_ = serialized.version();

        // Upgrade version
        if (version > original_version_) {
            LogError()("opentxs::storage::Node::")(__func__)(
                ": Upgrading to version ")(version)
                .Flush();
            version_ = version;
        } else {
            version_ = original_version_;
        }
    }
    auto delete_item(const UnallocatedCString& id) -> bool;
    auto delete_item(const Lock& lock, const UnallocatedCString& id) -> bool;
    auto set_alias(
        const UnallocatedCString& id,
        const UnallocatedCString& alias) -> bool;
    void set_hash(
        const VersionNumber version,
        const UnallocatedCString& id,
        const UnallocatedCString& hash,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const;
    auto store_raw(
        const UnallocatedCString& data,
        const UnallocatedCString& id,
        const UnallocatedCString& alias) -> bool;
    auto store_raw(
        const Lock& lock,
        const UnallocatedCString& data,
        const UnallocatedCString& id,
        const UnallocatedCString& alias) -> bool;
    auto verify_write_lock(const Lock& lock) const -> bool;

    virtual void init(const UnallocatedCString& hash) = 0;

    Node(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& key);
};
}  // namespace opentxs::storage
