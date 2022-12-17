// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageNymList.pb.h>
#include <mutex>

#include "opentxs/util/Container.hpp"
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

namespace storage
{
class Driver;
class Nym;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class Mailbox final : public Node
{
public:
    auto Load(
        const UnallocatedCString& id,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto Store(
        const UnallocatedCString& id,
        const UnallocatedCString& data,
        const UnallocatedCString& alias) -> bool;

    Mailbox() = delete;
    Mailbox(const Mailbox&) = delete;
    Mailbox(Mailbox&&) = delete;
    auto operator=(const Mailbox&) -> Mailbox = delete;
    auto operator=(Mailbox&&) -> Mailbox = delete;

    ~Mailbox() final = default;

private:
    friend Nym;

    void init(const UnallocatedCString& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;

    Mailbox(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& hash);
};
}  // namespace opentxs::storage
