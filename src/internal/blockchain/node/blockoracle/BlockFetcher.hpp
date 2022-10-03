// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>

#include "internal/network/zeromq/Types.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
namespace internal
{
class BlockBatch;
}  // namespace internal

class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class BlockFetcher final : public Allocated
{
public:
    class Actor;
    class Shared;

    auto GetJob(allocator_type alloc) const noexcept -> internal::BlockBatch;
    auto get_allocator() const noexcept -> allocator_type final;

    auto Init(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void;

    BlockFetcher(const api::Session& api, const node::Manager& node) noexcept;
    BlockFetcher() = delete;
    BlockFetcher(const BlockFetcher&) = delete;
    BlockFetcher(BlockFetcher&&) = delete;
    auto operator=(const BlockFetcher&) -> BlockFetcher& = delete;
    auto operator=(BlockFetcher&&) -> BlockFetcher& = delete;

    ~BlockFetcher() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Shared> shared_;

    BlockFetcher(
        const api::Session& api,
        const node::Manager& node,
        network::zeromq::BatchID batchID) noexcept;
    BlockFetcher(
        const api::Session& api,
        const node::Manager& node,
        network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::blockoracle
