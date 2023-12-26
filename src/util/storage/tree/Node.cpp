// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Node.hpp"  // IWYU pragma: associated

#include <Contact.pb.h>
#include <Nym.pb.h>
#include <Seed.pb.h>
#include <StorageItemHash.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string_view>

#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Node::Node(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& key,
    UnallocatedCString name,
    VersionNumber desired)
    : crypto_(crypto)
    , factory_(factory)
    , plugin_(storage)
    , name_(std::move(name))
    , desired_version_(desired)
    , original_version_()
    , version_()
    , root_(key)
    , write_lock_()
    , item_map_()
{
}

auto Node::blank() noexcept -> void
{
    set_original_version(desired_version_);
    root_ = NullHash{};
}

auto Node::copy(const Log& log, const Index& in, Vector<Hash>& out)
    const noexcept -> void
{
    out.reserve(out.size() + in.size());
    const auto get_hash = [&, this](const auto& i) {
        const auto& hash = std::get<0>(i.second);
        log()(name_)("adding item hash ")(hash).Flush();

        return hash;
    };
    std::ranges::transform(in, std::back_inserter(out), get_hash);
}

auto Node::delete_item(const identifier::Generic& id) -> bool
{
    auto lock = Lock{write_lock_};

    return delete_item(lock, id);
}

auto Node::delete_item(const Lock& lock, const identifier::Generic& id) -> bool
{
    assert_true(verify_write_lock(lock));

    const auto items = item_map_.erase(id);

    if (0 == items) { return false; }

    return save(lock);
}

auto Node::Dump(Vector<Hash>& out) const noexcept -> bool
{
    const auto& log = LogTrace();
    auto lock = Lock{write_lock_};

    return dump(lock, log, out);
}

auto Node::dump(const Lock& lock, const Log& log, Vector<Hash>& out)
    const noexcept -> bool
{
    if (is_valid(root_)) {
        log(name_)("copying root hash ")(root_).Flush();
        out.emplace_back(root_);
    } else {
        log(name_)("skipping object due to blank root hash").Flush();

        return true;
    }

    copy(log, item_map_, out);

    return true;
}

auto Node::extract_revision(const proto::Contact& input) const -> std::uint64_t
{
    return input.revision();
}

auto Node::extract_revision(const proto::Nym& input) const -> std::uint64_t
{
    return input.revision();
}

auto Node::extract_revision(const proto::Seed& input) const -> std::uint64_t
{
    return input.index();
}

auto Node::get_alias(const identifier::Generic& id) const -> UnallocatedCString
{
    UnallocatedCString output;
    const auto lock = Lock{write_lock_};
    const auto& it = item_map_.find(id);

    if (item_map_.end() != it) { output = std::get<1>(it->second); }

    return output;
}

auto Node::init_map(
    const google::protobuf::RepeatedPtrField<proto::StorageItemHash>&
        items) noexcept -> void
{
    auto lock = Lock{write_lock_};
    init_map(lock, items);
}

auto Node::init_map(
    const Lock& lock,
    const google::protobuf::RepeatedPtrField<proto::StorageItemHash>&
        items) noexcept -> void
{
    const auto load_metadata = [this, &lock](const auto& proto) {
        init_map(lock, proto);
    };
    std::ranges::for_each(items, load_metadata);
}

auto Node::init_map(
    const google::protobuf::RepeatedPtrField<proto::StorageItemHash>& in,
    Index& out) noexcept -> void
{
    const auto load_metadata = [this, &out](const auto& proto) {
        init_map(proto, out);
    };
    std::ranges::for_each(in, load_metadata);
}

auto Node::init_map(const Lock&, const proto::StorageItemHash& item) noexcept
    -> void
{
    init_map(item, item_map_);
}

auto Node::init_map(const proto::StorageItemHash& in, Index& out) const noexcept
    -> void
{
    out.emplace(
        [&] {
            if (in.has_id()) {

                return factory_.Internal().Identifier(in.id());
            } else {

                return factory_.IdentifierFromBase58(in.item_id_base58());
            }
        }(),
        Metadata{read(in.hash()), in.alias(), 0, false});
}

auto Node::List() const -> ObjectList
{
    ObjectList output;
    auto lock = Lock{write_lock_};

    for (const auto& it : item_map_) {
        output.emplace_back(it.first.asBase58(crypto_), std::get<1>(it.second));
    }

    lock.unlock();

    return output;
}

auto Node::load_raw(
    const identifier::Generic& id,
    UnallocatedCString& output,
    UnallocatedCString& alias,
    ErrorReporting checking) const -> bool
{
    const auto lock = Lock{write_lock_};
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

    return plugin_.Load(std::get<0>(it->second), checking, writer(output));
}

auto Node::Root() const -> Hash
{
    const auto lock = Lock{write_lock_};

    return root_;
}

auto Node::serialize_index(
    const identifier::Generic& id,
    const Hash& hash,
    ReadView alias,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const noexcept -> void
{
    set_hash(id, hash, output, type);
    output.set_alias(alias.data(), alias.size());
}

auto Node::serialize_index(
    const identifier::Generic& id,
    const Metadata& metadata,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const -> void
{
    serialize_index(
        id, std::get<0>(metadata), std::get<1>(metadata), output, type);
}

auto Node::set_alias(const identifier::Generic& id, std::string_view value)
    -> bool
{
    auto lock = Lock{write_lock_};

    if (auto it = item_map_.find(id); item_map_.end() != it) {
        auto& [hash, alias, revision, b] = it->second;
        auto old = alias;
        alias = value;

        if (false == save(lock)) {
            alias.swap(old);
            LogError()()("Failed to save node").Flush();

            return false;
        }

        return true;
    }

    LogError()()("item ")(id, crypto_)(" does not exist").Flush();

    return false;
}

auto Node::set_hash(
    const identifier::Generic& id,
    const Hash& hash,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const noexcept -> void
{
    output.set_version(storage_item_hash_version_);
    id.Internal().Serialize(*output.mutable_id());

    if (false == is_valid(hash)) {
        write(NullHash{}, *output.mutable_hash());
    } else {
        write(hash, *output.mutable_hash());
    }

    output.set_type(type);
}

auto Node::set_original_version(VersionNumber version) noexcept -> VersionNumber
{
    original_version_.set_value(version);
    version_.store(version);

    return version;
}

auto Node::store_raw(
    const UnallocatedCString& data,
    const identifier::Generic& id,
    std::string_view alias) -> bool
{
    auto lock = Lock{write_lock_};

    return store_raw(lock, data, id, alias);
}

auto Node::store_raw(
    const Lock& lock,
    const UnallocatedCString& data,
    const identifier::Generic& id,
    std::string_view alias) -> bool
{
    assert_true(verify_write_lock(lock));

    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (!plugin_.Store(data, hash)) { return false; }

    if (!alias.empty()) { std::get<1>(metadata) = alias; }

    return save(lock);
}

auto Node::Upgrade() noexcept -> bool
{
    auto lock = Lock{write_lock_};
    const auto changed = upgrade(lock);

    if (changed && (false == save(lock))) {
        LogAbort()()(name_)("save failure").Abort();
    }

    return changed;
}

auto Node::upgrade(const Lock&) noexcept -> bool
{
    version_.store(desired_version_);

    return original_version_.get() != desired_version_;
}

auto Node::UpgradeLevel() const -> VersionNumber { return original_version_; }

auto Node::verify_write_lock(const Lock& lock) const -> bool
{
    if (lock.mutex() != &write_lock_) {
        LogError()()("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()()("Lock not owned.").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::storage::tree
