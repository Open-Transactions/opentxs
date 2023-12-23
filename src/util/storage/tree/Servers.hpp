// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageServers.pb.h>
#include <memory>
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
class Notary;
}  // namespace identifier

namespace proto
{
class ServerContract;
}  // namespace proto

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
class Servers final : public Node
{
public:
    auto Alias(const identifier::Notary& id) const -> UnallocatedCString;
    auto Load(
        const identifier::Notary& id,
        std::shared_ptr<proto::ServerContract>& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;

    auto Delete(const identifier::Notary& id) -> bool;
    auto SetAlias(const identifier::Notary& id, std::string_view alias) -> bool;
    auto Store(
        const proto::ServerContract& data,
        std::string_view alias,
        UnallocatedCString& plaintext) -> bool;

    Servers() = delete;
    Servers(const Servers&) = delete;
    Servers(Servers&&) = delete;
    auto operator=(const Servers&) -> Servers = delete;
    auto operator=(Servers&&) -> Servers = delete;

    ~Servers() final = default;

private:
    friend Trunk;

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageServers;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Servers(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash);
};
}  // namespace opentxs::storage::tree
