// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string_view>

#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace node
{
namespace wallet
{
class ReorgSlavePrivate;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Pipeline;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class ReorgSlave final : public Allocated, public Reorg
{
public:
    auto get_allocator() const noexcept -> allocator_type final;

    auto AcknowledgePrepareReorg(Reorg::Job&& job) noexcept -> void;
    auto AcknowledgeShutdown() noexcept -> void;
    [[nodiscard]] auto GetSlave(
        const network::zeromq::Pipeline& parent,
        std::string_view name,
        alloc::Strategy alloc) noexcept -> ReorgSlave final;
    [[nodiscard]] auto Start() noexcept -> bool;
    auto Stop() noexcept -> void;

    ReorgSlave(boost::shared_ptr<ReorgSlavePrivate> imp) noexcept;
    ReorgSlave(const ReorgSlave&) = delete;
    ReorgSlave(ReorgSlave&& rhs) noexcept;
    auto operator=(const ReorgSlave&) -> ReorgSlave& = delete;
    auto operator=(ReorgSlave&&) -> ReorgSlave& = delete;

    ~ReorgSlave() final;

private:
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<ReorgSlavePrivate> imp_;
};
}  // namespace opentxs::blockchain::node::wallet
