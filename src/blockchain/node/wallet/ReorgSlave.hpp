// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <string_view>

#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace node
{
namespace wallet
{
class ReorgMasterPrivate;
class ReorgSlave;
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

class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
namespace opentxs::blockchain::node::wallet
{
class ReorgSlavePrivate final : public Allocated,
                                public boost::enable_shared_from
{
public:
    const Log& log_;
    const CString name_;

    auto get_allocator() const noexcept -> allocator_type final;

    auto AcknowledgePrepareReorg(Reorg::Job&& job) noexcept -> void;
    auto AcknowledgeShutdown() noexcept -> void;
    auto BroadcastFinishReorg() noexcept -> void;
    auto BroadcastPrepareReorg(StateSequence id) noexcept -> void;
    auto BroadcastPrepareShutdown() noexcept -> void;
    auto BroadcastShutdown() noexcept -> void;
    [[nodiscard]] auto GetSlave(
        const network::zeromq::Pipeline& parent,
        std::string_view name,
        allocator_type alloc) noexcept -> ReorgSlave;
    auto Start() noexcept -> Reorg::State;
    auto Stop() noexcept -> void;

    ReorgSlavePrivate(
        const network::zeromq::Pipeline& parent,
        boost::shared_ptr<ReorgMasterPrivate> master,
        std::string_view name,
        allocator_type alloc) noexcept;
    ReorgSlavePrivate(const ReorgSlavePrivate&) = delete;
    ReorgSlavePrivate(ReorgSlavePrivate&&) = delete;
    auto operator=(const ReorgSlavePrivate&) -> ReorgSlavePrivate& = delete;
    auto operator=(ReorgSlavePrivate&&) -> ReorgSlavePrivate& = delete;

    ~ReorgSlavePrivate() final;

private:
    const network::zeromq::Pipeline& parent_;
    boost::shared_ptr<ReorgMasterPrivate> master_;
    int id_;
    allocator_type alloc_;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::blockchain::node::wallet