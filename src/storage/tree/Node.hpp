// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

#define OT_METHOD "opentxs::Node::"

namespace opentxs
{
namespace storage
{
class Root;

typedef std::function<bool(const std::string&)> keyFunction;
/** A set of metadata associated with a stored object
 *  * string: hash
 *  * string: alias
 *  * uint64: revision
 *  * bool:   private
 */
typedef std::tuple<std::string, std::string, std::uint64_t, bool> Metadata;
/** Maps a logical id to the stored metadata for the object
 *  * string: id of the stored object
 *  * Metadata: metadata for the stored object
 */
typedef std::map<std::string, Metadata> Index;

class Node
{
protected:
    template <class T>
    bool store_proto(
        const Lock& lock,
        const T& data,
        const std::string& id,
        const std::string& alias,
        std::string& plaintext)
    {
        OT_ASSERT(verify_write_lock(lock))

        auto& metadata = item_map_[id];
        auto& hash = std::get<0>(metadata);

        if (!driver_.StoreProto<T>(data, hash, plaintext)) { return false; }

        if (!alias.empty()) { std::get<1>(metadata) = alias; }

        return save(lock);
    }

    template <class T>
    bool store_proto(
        const T& data,
        const std::string& id,
        const std::string& alias,
        std::string& plaintext)
    {
        Lock lock(write_lock_);

        return store_proto(lock, data, id, alias, plaintext);
    }

    template <class T>
    bool store_proto(
        const T& data,
        const std::string& id,
        const std::string& alias)
    {
        std::string notUsed;

        return store_proto<T>(data, id, alias, notUsed);
    }

    template <class T>
    bool load_proto(
        const std::string& id,
        std::shared_ptr<T>& output,
        std::string& alias,
        const bool checking) const
    {
        Lock lock(write_lock_);
        const auto& it = item_map_.find(id);
        const bool exists = (item_map_.end() != it);

        if (!exists) {
            if (!checking) {
                std::cout << __FUNCTION__ << ": Error: item with id " << id
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
    bool check_revision(
        const std::string& method,
        const std::uint64_t incoming,
        Metadata& metadata)
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
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to load object.")
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
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = delete;

protected:
    friend class Root;

    static const std::string BLANK_HASH;

    const opentxs::api::storage::Driver& driver_;

    std::uint32_t version_{0};
    std::uint32_t original_version_{0};
    mutable std::string root_;

    mutable std::mutex write_lock_;
    mutable Index item_map_;

    static std::string normalize_hash(const std::string& hash);

    bool check_hash(const std::string& hash) const;
    std::uint64_t extract_revision(const proto::Contact& input) const;
    std::uint64_t extract_revision(const proto::CredentialIndex& input) const;
    std::uint64_t extract_revision(const proto::Seed& input) const;
    std::string get_alias(const std::string& id) const;
    bool load_raw(
        const std::string& id,
        std::string& output,
        std::string& alias,
        const bool checking) const;
    bool migrate(
        const std::string& hash,
        const opentxs::api::storage::Driver& to) const;
    virtual bool save(const Lock& lock) const = 0;
    void serialize_index(
        const std::string& id,
        const Metadata& metadata,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const;

    bool delete_item(const std::string& id);
    bool delete_item(const Lock& lock, const std::string& id);
    bool set_alias(const std::string& id, const std::string& alias);
    void set_hash(
        const std::uint32_t version,
        const std::string& id,
        const std::string& hash,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const;
    bool store_raw(
        const std::string& data,
        const std::string& id,
        const std::string& alias);
    bool store_raw(
        const Lock& lock,
        const std::string& data,
        const std::string& id,
        const std::string& alias);
    bool verify_write_lock(const Lock& lock) const;

    virtual void init(const std::string& hash) = 0;

    Node(const opentxs::api::storage::Driver& storage, const std::string& key);

public:
    virtual ObjectList List() const;
    virtual bool Migrate(const opentxs::api::storage::Driver& to) const;
    std::string Root() const;
    std::uint32_t UpgradeLevel() const;

    virtual ~Node() = default;
};
}  // namespace storage
}  // namespace opentxs
