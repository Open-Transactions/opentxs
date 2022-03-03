// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/StorageServers.pb.h"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class ServerContract;
}  // namespace proto

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Servers final : public Node
{
private:
    friend Tree;

    void init(const UnallocatedCString& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageServers;

    Servers(const Driver& storage, const UnallocatedCString& hash);
    Servers() = delete;
    Servers(const Servers&) = delete;
    Servers(Servers&&) = delete;
    auto operator=(const Servers&) -> Servers = delete;
    auto operator=(Servers&&) -> Servers = delete;

public:
    auto Alias(const UnallocatedCString& id) const -> UnallocatedCString;
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::ServerContract>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;
    void Map(ServerLambda lambda) const;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto SetAlias(const UnallocatedCString& id, const UnallocatedCString& alias)
        -> bool;
    auto Store(
        const proto::ServerContract& data,
        const UnallocatedCString& alias,
        UnallocatedCString& plaintext) -> bool;

    ~Servers() final = default;
};
}  // namespace opentxs::storage
