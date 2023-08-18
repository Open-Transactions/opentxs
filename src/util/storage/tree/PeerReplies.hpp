// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageNymList.pb.h>
#include <memory>
#include <mutex>

#include "internal/util/Mutex.hpp"
#include "internal/util/storage/Types.hpp"
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
class Generic;
}  // namespace identifier

namespace proto
{
class PeerReply;
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
class PeerReplies final : public Node
{
public:
    auto Load(
        const identifier::Generic& id,
        std::shared_ptr<proto::PeerReply>& output,
        ErrorReporting checking) const -> bool;

    auto Delete(const identifier::Generic& id) -> bool;
    auto Store(const proto::PeerReply& data) -> bool;

    PeerReplies() = delete;
    PeerReplies(const PeerReplies&) = delete;
    PeerReplies(PeerReplies&&) = delete;
    auto operator=(const PeerReplies&) -> PeerReplies = delete;
    auto operator=(PeerReplies&&) -> PeerReplies = delete;

    ~PeerReplies() final = default;

private:
    friend Nym;

    auto init(const Hash& hash) noexcept(false) -> void final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;
    auto upgrade(const Lock& lock) noexcept -> bool final;

    PeerReplies(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const driver::Plugin& storage,
        const Hash& hash);
};
}  // namespace opentxs::storage::tree
