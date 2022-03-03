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
#include "serialization/protobuf/StorageUnits.pb.h"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class UnitDefinition;
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
class Units final : public Node
{
private:
    friend Tree;

    void init(const UnallocatedCString& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageUnits;

    Units(const Driver& storage, const UnallocatedCString& key);
    Units() = delete;
    Units(const Units&) = delete;
    Units(Units&&) = delete;
    auto operator=(const Units&) -> Units = delete;
    auto operator=(Units&&) -> Units = delete;

public:
    auto Alias(const UnallocatedCString& id) const -> UnallocatedCString;
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::UnitDefinition>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;
    void Map(UnitLambda lambda) const;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto SetAlias(const UnallocatedCString& id, const UnallocatedCString& alias)
        -> bool;
    auto Store(
        const proto::UnitDefinition& data,
        const UnallocatedCString& alias,
        UnallocatedCString& plaintext) -> bool;

    ~Units() final = default;
};
}  // namespace opentxs::storage
