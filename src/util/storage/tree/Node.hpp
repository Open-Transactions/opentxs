// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageEnums.pb.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <source_location>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/Types.hpp"
#include "internal/util/storage/drivers/Plugin.hpp"
#include "internal/util/storage/tree/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"
#include "opentxs/util/storage/Types.hpp"

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
namespace tree
{
class Root;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
using enum ErrorReporting;

class Node
{
public:
    auto Dump(Vector<Hash>& out) const noexcept -> bool;
    virtual auto List() const -> ObjectList;
    auto Root() const -> Hash;
    auto UpgradeLevel() const -> VersionNumber;

    auto Upgrade() noexcept -> bool;

    Node() = delete;
    Node(const Node&) = delete;
    Node(Node&&) = delete;
    auto operator=(const Node&) -> Node& = delete;
    auto operator=(Node&&) -> Node& = delete;

    virtual ~Node() = default;

protected:
    template <class T>
    auto LoadProto(
        const Hash& hash,
        std::shared_ptr<T>& serialized,
        ErrorReporting checking) const noexcept -> bool
    {
        auto raw = UnallocatedCString{};
        const auto loaded = plugin_.Load(hash, checking, writer(raw), nullptr);
        auto valid{false};

        if (loaded) {
            serialized = proto::DynamicFactory<T>(raw.data(), raw.size());

            assert_false(nullptr == serialized);

            valid = proto::Validate<T>(*serialized, VERBOSE);
        } else {

            return false;
        }

        if (!valid) {
            if (loaded) {
                LogError()(": Specified object was located but could not be "
                           "validated.")
                    .Flush();
                LogError()(": Hash: ")(hash).Flush();
                LogError()(": Size: ")(raw.size()).Flush();
            } else {

                LogDetail()("Specified object is missing.").Flush();
                LogDetail()("Hash: ")(hash).Flush();
                LogDetail()("Size: ")(raw.size()).Flush();
            }
        }

        assert_true(valid);

        return valid;
    }

    template <class T>
    auto StoreProto(const T& data, Hash& key, UnallocatedCString& plaintext)
        const noexcept -> bool
    {
        if (false == proto::Validate<T>(data, VERBOSE)) { return false; }

        plaintext = proto::ToString(data);

        return plugin_.Store(plaintext, key);
    }

    template <class T>
    auto StoreProto(const T& data, Hash& key) const noexcept -> bool
    {
        auto notUsed = UnallocatedCString{};

        return StoreProto<T>(data, key, notUsed);
    }

protected:
    template <class T>
    auto check_revision(
        const std::uint64_t incoming,
        Metadata& metadata,
        const std::source_location& loc =
            std::source_location::current()) const noexcept -> bool
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
            using enum ErrorReporting;

            if (false == LoadProto(hash, existing, verbose)) {
                LogAbort()(loc)(": Unable to load object.").Abort();
            }

            revision = extract_revision(*existing);
        }

        return (incoming > revision);
    }
    template <class T>
    auto load_proto(
        const identifier::Generic& id,
        std::shared_ptr<T>& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const noexcept -> bool
    {
        if (id.empty()) { return false; }

        Lock lock(write_lock_);
        const auto& it = item_map_.find(id);
        const bool exists = (item_map_.end() != it);

        if (false == exists) {
            using enum ErrorReporting;

            if (verbose == checking) {
                LogError()()("Error: item with id ")(id, crypto_)(
                    " does not exist.")
                    .Flush();
            }

            return false;
        }

        alias = std::get<1>(it->second);

        return LoadProto<T>(std::get<0>(it->second), output, checking);
    }

    template <class T>
    auto store_proto(
        const Lock& lock,
        const T& data,
        const identifier::Generic& id,
        std::string_view alias,
        UnallocatedCString& plaintext) noexcept -> bool
    {
        assert_true(verify_write_lock(lock));

        auto& metadata = item_map_[id];
        auto& hash = std::get<0>(metadata);

        if (false == StoreProto<T>(data, hash, plaintext)) { return false; }

        if (false == alias.empty()) { std::get<1>(metadata) = alias; }

        return save(lock);
    }
    template <class T>
    auto store_proto(
        const T& data,
        const identifier::Generic& id,
        std::string_view alias,
        UnallocatedCString& plaintext) noexcept -> bool
    {
        Lock lock(write_lock_);

        return store_proto(lock, data, id, alias, plaintext);
    }
    template <class T>
    auto store_proto(
        const T& data,
        const identifier::Generic& id,
        std::string_view alias) noexcept -> bool
    {
        UnallocatedCString notUsed;

        return store_proto<T>(data, id, alias, notUsed);
    }

protected:
    friend tree::Root;

    static constexpr auto storage_item_hash_version_ = VersionNumber{2};

    const api::Crypto& crypto_;
    const api::session::Factory& factory_;
    const driver::Plugin& plugin_;
    const UnallocatedCString name_;
    const VersionNumber desired_version_;
    DeferredConstruction<VersionNumber> original_version_;
    std::atomic<VersionNumber> version_;
    mutable Hash root_;
    mutable std::mutex write_lock_;
    mutable Index item_map_;

    auto copy(const Log& log, const Index& in, Vector<Hash>& out) const noexcept
        -> void;
    virtual auto dump(const Lock&, const Log&, Vector<Hash>& out) const noexcept
        -> bool;
    auto extract_revision(const proto::Contact& input) const -> std::uint64_t;
    auto extract_revision(const proto::Nym& input) const -> std::uint64_t;
    auto extract_revision(const proto::Seed& input) const -> std::uint64_t;
    auto get_alias(const identifier::Generic& id) const -> UnallocatedCString;
    auto load_raw(
        const identifier::Generic& id,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;
    virtual auto save(const Lock& lock) const -> bool = 0;
    auto serialize_index(
        const identifier::Generic& id,
        const Hash& hash,
        ReadView alias,
        proto::StorageItemHash& output,
        const proto::StorageHashType type =
            proto::STORAGEHASH_PROTO) const noexcept -> void;
    auto serialize_index(
        const identifier::Generic& id,
        const Metadata& metadata,
        proto::StorageItemHash& output,
        const proto::StorageHashType type = proto::STORAGEHASH_PROTO) const
        -> void;

    virtual auto blank() noexcept -> void;
    auto delete_item(const identifier::Generic& id) -> bool;
    auto delete_item(const Lock& lock, const identifier::Generic& id) -> bool;
    auto set_alias(const identifier::Generic& id, std::string_view alias)
        -> bool;
    auto set_hash(
        const identifier::Generic& id,
        const Hash& hash,
        proto::StorageItemHash& output,
        const proto::StorageHashType type =
            proto::STORAGEHASH_PROTO) const noexcept -> void;
    auto store_raw(
        const UnallocatedCString& data,
        const identifier::Generic& id,
        std::string_view alias) -> bool;
    auto store_raw(
        const Lock& lock,
        const UnallocatedCString& data,
        const identifier::Generic& id,
        std::string_view alias) -> bool;
    auto verify_write_lock(const Lock& lock) const -> bool;

    virtual void init(const Hash& hash) = 0;
    auto init_map(
        const google::protobuf::RepeatedPtrField<proto::StorageItemHash>&
            items) noexcept -> void;
    auto init_map(
        const Lock& lock,
        const google::protobuf::RepeatedPtrField<proto::StorageItemHash>&
            items) noexcept -> void;
    auto init_map(const Lock& lock, const proto::StorageItemHash& item) noexcept
        -> void;
    auto init_map(
        const google::protobuf::RepeatedPtrField<proto::StorageItemHash>& in,
        Index& out) noexcept -> void;
    auto init_map(const proto::StorageItemHash& in, Index& out) const noexcept
        -> void;
    auto set_original_version(VersionNumber version) noexcept -> VersionNumber;
    virtual auto upgrade(const Lock& lock) noexcept -> bool = 0;

    Node(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& root,
        UnallocatedCString name,
        VersionNumber desired) noexcept(false);
};
}  // namespace opentxs::storage::tree
