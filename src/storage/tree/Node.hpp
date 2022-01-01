// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/StorageEnums.pb.h"
#include "storage/Plugin.hpp"

namespace opentxs
{
namespace proto
{
class Contact;
class Nym;
class Seed;
class StorageItemHash;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace storage
{
class Driver;
class Root;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
using keyFunction = std::function<bool(const std::string&)>;
/** A set of metadata associated with a stored object
 *  * string: hash
 *  * string: alias
 *  * uint64: revision
 *  * bool:   private
 */
using Metadata = std::tuple<std::string, std::string, std::uint64_t, bool>;
/** Maps a logical id to the stored metadata for the object
 *  * string: id of the stored object
 *  * Metadata: metadata for the stored object
 */
using Index = std::pmr::map<std::string, Metadata>;

class Node
{
protected:
    template <class T>
    auto store_proto(
        const Lock& lock,
        const T& data,
        const std::string& id,
        const std::string& alias,
        std::string& plaintext) -> bool
    {
        OT_ASSERT(verify_write_lock(lock))

        auto& metadata = item_map_[id];
        auto& hash = std::get<0>(metadata);

        if (!driver_.StoreProto<T>(data, hash, plaintext)) { return false; }

        if (!alias.empty()) { std::get<1>(metadata) = alias; }

        return save(lock);
    }

    template <class T>
    auto store_proto(
        const T& data,
        const std::string& id,
        const std::string& alias,
        std::string& plaintext) -> bool
    {
        Lock lock(write_lock_);

        return store_proto(lock, data, id, alias, plaintext);
    }

    template <class T>
    auto store_proto(
        const T& data,
        const std::string& id,
        const std::string& alias) -> bool
    {
        std::string notUsed;

        return store_proto<T>(data, id, alias, notUsed);
    }

    template <class T>
    auto load_proto(
        const std::string& id,
        std::shared_ptr<T>& output,
        std::string& alias,
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
        const std::string& method,
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

private:
    Node() = delete;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    auto operator=(const Node&) -> Node& = delete;
    auto operator=(Node&&) -> Node& = delete;

protected:
    friend storage::Root;

    static const std::string BLANK_HASH;

    const Driver& driver_;
    VersionNumber version_;
    VersionNumber original_version_;
    mutable std::string root_;
    mutable std::mutex write_lock_;
    mutable Index item_map_;

    static auto normalize_hash(const std::string& hash) -> std::string;

    auto check_hash(const std::string& hash) const -> bool;
    auto extract_revision(const proto::Contact& input) const -> std::uint64_t;
    auto extract_revision(const proto::Nym& input) const -> std::uint64_t;
    auto extract_revision(const proto::Seed& input) const -> std::uint64_t;
    auto get_alias(const std::string& id) const -> std::string;
    auto load_raw(
        const std::string& id,
        std::string& output,
        std::string& alias,
        const bool checking) const -> bool;
    auto migrate(const std::string& hash, const Driver& to) const -> bool;
    virtual auto save(const Lock& lock) const -> bool = 0;
    void serialize_index(
        const VersionNumber version,
        const std::string& id,
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
    auto delete_item(const std::string& id) -> bool;
    auto delete_item(const Lock& lock, const std::string& id) -> bool;
    auto set_alias(const std::string& id, const std::string& alias) -> bool;
    void set_hash(
        const VersionNumber version,
        const std::string& id,
        const std::string& hash,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const;
    auto store_raw(
        const std::string& data,
        const std::string& id,
        const std::string& alias) -> bool;
    auto store_raw(
        const Lock& lock,
        const std::string& data,
        const std::string& id,
        const std::string& alias) -> bool;
    auto verify_write_lock(const Lock& lock) const -> bool;

    virtual void init(const std::string& hash) = 0;

    Node(const Driver& storage, const std::string& key);

public:
    virtual auto List() const -> ObjectList;
    virtual auto Migrate(const Driver& to) const -> bool;
    auto Root() const -> std::string;
    auto UpgradeLevel() const -> VersionNumber;

    virtual ~Node() = default;
};
}  // namespace opentxs::storage
