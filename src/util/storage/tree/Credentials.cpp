// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Credentials.hpp"  // IWYU pragma: associated

#include <Credential.pb.h>
#include <Enums.pb.h>
#include <StorageCredentials.pb.h>
#include <atomic>
#include <source_location>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/crypto/key/Key.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Credential.hpp"
#include "internal/serialization/protobuf/verify/StorageCredentials.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/asymmetric/Mode.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Credentials::Credentials(
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
          2)
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Credentials::Alias(const identifier::Generic& id) const
    -> UnallocatedCString
{
    return get_alias(id);
}

auto Credentials::check_existing(const bool incoming, Metadata& metadata) const
    -> bool
{
    const auto& hash = std::get<0>(metadata);
    auto& isPrivate = std::get<3>(metadata);

    if (incoming) {
        // If the credential to be saved is private, we're going to save it
        // regardless of the state of the existing version.
        isPrivate = true;

        return isPrivate;
    }

    // This variable can be false for two reasons:
    // * The stored version is public,
    // * It's private but hasn't been loaded yet and so the index
    // hasn't been updated
    // ...so we have to load the credential just to be sure
    if (!isPrivate) {
        std::shared_ptr<proto::Credential> existing;
        using enum ErrorReporting;

        if (!LoadProto(hash, existing, verbose)) {
            LogAbort()()("Failed to load object").Abort();
        }

        isPrivate =
            (crypto::asymmetric::Mode::Private == translate(existing->mode()));
    }

    return !isPrivate;
}

auto Credentials::Delete(const identifier::Generic& id) -> bool
{
    return delete_item(id);
}

auto Credentials::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageCredentials>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 2u:
            case 1u:
            default: {
                init_map(proto.cred());
            }
        }
    } else {
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto Credentials::Load(
    const identifier::Generic& id,
    std::shared_ptr<proto::Credential>& cred,
    ErrorReporting checking) const -> bool
{
    const auto lock = Lock{write_lock_};
    const bool exists = (item_map_.end() != item_map_.find(id));

    if (false == exists) {
        using enum ErrorReporting;

        if (verbose == checking) {
            LogError()()("credential with id ")
                .asHex(id)(" does not exist.")
                .Flush();
        }

        return false;
    }

    auto& metadata = item_map_[id];
    const auto& hash = std::get<0>(metadata);
    auto& isPrivate = std::get<3>(metadata);
    const bool loaded = LoadProto(hash, cred, checking);

    if (!loaded) { return false; }

    isPrivate = (crypto::asymmetric::Mode::Private == translate(cred->mode()));

    return true;
}

auto Credentials::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) { LogAbort()()("Lock failure").Abort(); }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return StoreProto(serialized, root_);
}

auto Credentials::serialize() const -> proto::StorageCredentials
{
    proto::StorageCredentials serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_cred());
        }
    }

    return serialized;
}

auto Credentials::SetAlias(
    const identifier::Generic& id,
    std::string_view alias) -> bool
{
    return set_alias(id, alias);
}

auto Credentials::Store(const proto::Credential& cred, std::string_view alias)
    -> bool
{
    const auto lock = Lock{write_lock_};
    const auto id = factory_.Internal().Identifier(cred.id());
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    const bool incomingPrivate = (proto::KEYMODE_PRIVATE == cred.mode());
    const bool incomingPublic = (proto::KEYMODE_PUBLIC == cred.mode());

    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey && incomingPublic) {
        if (!check_existing(incomingPrivate, metadata)) {
            // We're trying to save a public credential but already have
            // the private version saved. Just silently skip this update
            // instead of overwriting private keys.

            return true;
        }
    }

    if (!StoreProto(cred, hash)) { return false; }

    if (!alias.empty()) { std::get<1>(metadata) = alias; }

    return save(lock);
}

auto Credentials::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        case 2u:
        default: {
        }
    }

    return changed;
}
}  // namespace opentxs::storage::tree
