// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/StorageSeeds.pb.h>
#include <memory>
#include <mutex>
#include <string_view>

#include "internal/util/Mutex.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/storage/tree/Node.hpp"

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

namespace protobuf
{
class Seed;
}  // namespace protobuf

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Trunk;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Seeds final : public Node
{
public:
    auto Alias(const opentxs::crypto::SeedID& id) const -> UnallocatedCString;
    auto Default() const -> opentxs::crypto::SeedID;
    auto Load(
        const opentxs::crypto::SeedID& id,
        std::shared_ptr<protobuf::Seed>& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;

    auto Delete(const opentxs::crypto::SeedID& id) -> bool;
    auto SetAlias(const opentxs::crypto::SeedID& id, std::string_view alias)
        -> bool;
    auto SetDefault(const opentxs::crypto::SeedID& id) -> bool;
    auto Store(const opentxs::crypto::SeedID& id, const protobuf::Seed& data)
        -> bool;

    Seeds() = delete;
    Seeds(const Seeds&) = delete;
    Seeds(Seeds&&) = delete;
    auto operator=(const Seeds&) -> Seeds = delete;
    auto operator=(Seeds&&) -> Seeds = delete;

    ~Seeds() final = default;

private:
    friend Trunk;

    static constexpr auto current_version_ = VersionNumber{2};

    opentxs::crypto::SeedID default_seed_;

    auto init(const Hash& hash) -> void final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto set_default(
        const std::unique_lock<std::mutex>& lock,
        const opentxs::crypto::SeedID& id) -> void;
    auto serialize() const -> protobuf::StorageSeeds;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Seeds(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash);
};
}  // namespace opentxs::storage::tree
