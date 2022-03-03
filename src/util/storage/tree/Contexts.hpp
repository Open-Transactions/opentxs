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
#include "serialization/protobuf/StorageNymList.pb.h"
#include "util/storage/tree/Node.hpp"

namespace opentxs
{
namespace proto
{
class Context;
class Driver;
}  // namespace proto

namespace storage
{
class Driver;
class Nym;

class Contexts final : public Node
{
private:
    friend Nym;

    void init(const UnallocatedCString& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;

    Contexts(const Driver& storage, const UnallocatedCString& hash);
    Contexts() = delete;
    Contexts(const Contexts&) = delete;
    Contexts(Contexts&&) = delete;
    auto operator=(const Contexts&) -> Contexts = delete;
    auto operator=(Contexts&&) -> Contexts = delete;

public:
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::Context>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto Store(const proto::Context& data, const UnallocatedCString& alias)
        -> bool;

    ~Contexts() final = default;
};
}  // namespace storage
}  // namespace opentxs
