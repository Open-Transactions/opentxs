// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageSeeds.pb.h>
#include <cstdint>
#include <memory>
#include <mutex>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Editor.hpp"
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

namespace proto
{
class Seed;
}  // namespace proto

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Seeds final : public Node
{
public:
    auto Alias(const UnallocatedCString& id) const -> UnallocatedCString;
    auto Default() const -> UnallocatedCString;
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::Seed>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto SetAlias(const UnallocatedCString& id, const UnallocatedCString& alias)
        -> bool;
    auto SetDefault(const UnallocatedCString& id) -> bool;
    auto Store(const proto::Seed& data) -> bool;

    Seeds() = delete;
    Seeds(const Seeds&) = delete;
    Seeds(Seeds&&) = delete;
    auto operator=(const Seeds&) -> Seeds = delete;
    auto operator=(Seeds&&) -> Seeds = delete;

    ~Seeds() final = default;

private:
    friend Tree;

    static constexpr auto current_version_ = VersionNumber{2};

    UnallocatedCString default_seed_;

    auto init(const UnallocatedCString& hash) -> void final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto set_default(
        const std::unique_lock<std::mutex>& lock,
        const UnallocatedCString& id) -> void;
    auto serialize() const -> proto::StorageSeeds;

    Seeds(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& hash);
};
}  // namespace opentxs::storage
