// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <StorageNymList.pb.h>
#include <memory>
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

namespace proto
{
class PeerRequest;
}  // namespace proto

namespace storage
{
class Driver;
class Nym;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
class PeerRequests final : public Node
{
public:
    auto Load(
        const UnallocatedCString& id,
        std::shared_ptr<proto::PeerRequest>& output,
        UnallocatedCString& alias,
        const bool checking) const -> bool;

    auto Delete(const UnallocatedCString& id) -> bool;
    auto SetAlias(const UnallocatedCString& id, const UnallocatedCString& alias)
        -> bool;
    auto Store(const proto::PeerRequest& data, const UnallocatedCString& alias)
        -> bool;

    PeerRequests() = delete;
    PeerRequests(const PeerRequests&) = delete;
    PeerRequests(PeerRequests&&) = delete;
    auto operator=(const PeerRequests&) -> PeerRequests = delete;
    auto operator=(PeerRequests&&) -> PeerRequests = delete;

    ~PeerRequests() final = default;

private:
    friend Nym;

    void init(const UnallocatedCString& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;

    PeerRequests(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const Driver& storage,
        const UnallocatedCString& hash);
};
}  // namespace opentxs::storage
