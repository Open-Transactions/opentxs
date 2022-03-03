// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/StorageCredentials.pb.h"
#include "util/storage/tree/Node.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class Credential;
}  // namespace proto

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
// }  // namespace v1
}  // namespace opentxs
// NOLINTBEGIN(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Credentials final : public Node
{
private:
    friend Tree;

    auto check_existing(const bool incoming, Metadata& metadata) const -> bool;
    void init(const UnallocatedCString& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageCredentials;

    Credentials(const Driver& storage, const UnallocatedCString& hash);
    Credentials() = delete;
    Credentials(const Credentials&) = delete;
    Credentials(Credentials&&) = delete;
    auto operator=(const Credentials&) -> Credentials = delete;
    auto operator=(Credentials&&) -> Credentials = delete;

public:
    auto Alias(const UnallocatedCString& id) const -> UnallocatedCString;
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::Credential>& output,
        const bool checking) const -> bool;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto SetAlias(const UnallocatedCString& id, const UnallocatedCString& alias)
        -> bool;
    auto Store(const proto::Credential& data, const UnallocatedCString& alias)
        -> bool;

    ~Credentials() final = default;
};
}  // namespace opentxs::storage
