// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageIssuers.pb.h>
#include <memory>
#include <mutex>
#include <string_view>

#include "internal/util/Mutex.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/storage/Types.hpp"
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
class Nym;
}  // namespace identifier

namespace proto
{
class Issuer;
}  // namespace proto

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
class Issuers final : public Node
{
public:
    auto Load(
        const identifier::Nym& id,
        std::shared_ptr<proto::Issuer>& output,
        UnallocatedCString& alias,
        ErrorReporting checking) const -> bool;

    auto Delete(const identifier::Nym& id) -> bool;
    auto Store(const proto::Issuer& data, std::string_view alias) -> bool;

    Issuers() = delete;
    Issuers(const Issuers&) = delete;
    Issuers(Issuers&&) = delete;
    auto operator=(const Issuers&) -> Issuers = delete;
    auto operator=(Issuers&&) -> Issuers = delete;

    ~Issuers() final = default;

private:
    friend Nym;

    static constexpr auto current_version_ = VersionNumber{1};

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageIssuers;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    Issuers(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash);
};
}  // namespace opentxs::storage::tree
