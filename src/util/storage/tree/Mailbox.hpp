// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/StorageNymList.pb.h>
#include <mutex>
#include <string_view>

#include "internal/util/Mutex.hpp"
#include "opentxs/storage/Types.hpp"
#include "opentxs/storage/Types.internal.hpp"
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

namespace identifier
{
class Generic;
}  // namespace identifier

namespace storage
{
namespace driver
{
class Plugin;
}  // namespace driver

namespace tree
{
class Nym;
}  // namespace tree
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Mailbox final : public Node
{
public:
    auto Load(
        const identifier::Generic& id,
        UnallocatedCString& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;

    auto Delete(const identifier::Generic& id) -> bool;
    auto Store(
        const identifier::Generic& id,
        const UnallocatedCString& data,
        std::string_view alias) -> bool;

    Mailbox() = delete;
    Mailbox(const Mailbox&) = delete;
    Mailbox(Mailbox&&) = delete;
    auto operator=(const Mailbox&) -> Mailbox = delete;
    auto operator=(Mailbox&&) -> Mailbox = delete;

    ~Mailbox() final = default;

private:
    friend Nym;

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> protobuf::StorageNymList;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Mailbox(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash);
};
}  // namespace opentxs::storage::tree
