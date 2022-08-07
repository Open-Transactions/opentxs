// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_plain_guarded.h>
#include <mutex>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace node
{
namespace wallet
{
class ReorgSlave;
class ReorgSlavePrivate;
}  // namespace wallet

class HeaderOracle;
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
class ReorgMasterPrivate final : public Allocated,
                                 public boost::enable_shared_from
{
public:
    using SlaveID = int;

    auto get_allocator() const noexcept -> allocator_type final;

    auto AcknowledgePrepareReorg(SlaveID id, Reorg::Job&& job) noexcept -> void;
    auto AcknowledgeShutdown(SlaveID id) noexcept -> void;
    auto CheckShutdown() noexcept -> bool;
    auto ClearReorg() noexcept -> void;
    auto FinishReorg() noexcept -> void;
    [[nodiscard]] auto GetReorg(
        const block::Position& position,
        storage::lmdb::LMDB::Transaction&& tx) noexcept -> Reorg::Params&;
    [[nodiscard]] auto GetSlave(
        const network::zeromq::Pipeline& parent,
        std::string_view name,
        allocator_type alloc) noexcept -> ReorgSlave;
    [[nodiscard]] auto PerformReorg(const node::HeaderOracle& oracle) noexcept
        -> bool;
    [[nodiscard]] auto PrepareReorg(StateSequence id) noexcept -> bool;
    [[nodiscard]] auto PrepareShutdown() noexcept -> bool;
    auto Register(boost::shared_ptr<ReorgSlavePrivate> slave) noexcept
        -> std::pair<SlaveID, Reorg::State>;
    auto Stop() noexcept -> void;
    auto Unregister(SlaveID id) noexcept -> void;

    ReorgMasterPrivate(
        const network::zeromq::Pipeline& parent,
        allocator_type alloc) noexcept;
    ReorgMasterPrivate(const ReorgMasterPrivate&) = delete;
    ReorgMasterPrivate(ReorgMasterPrivate&&) = delete;
    auto operator=(const ReorgMasterPrivate&) -> ReorgMasterPrivate& = delete;
    auto operator=(ReorgMasterPrivate&&) -> ReorgMasterPrivate& = delete;

    ~ReorgMasterPrivate() final;

private:
    struct Data {
        const network::zeromq::Pipeline& parent_;
        Reorg::State state_;
        SlaveID counter_;
        Map<SlaveID, boost::shared_ptr<ReorgSlavePrivate>> slaves_;
        Map<SlaveID, Reorg::Job> actions_;
        Set<SlaveID> acks_;
        std::optional<Reorg::Params> params_;

        Data(
            const network::zeromq::Pipeline& parent,
            allocator_type alloc) noexcept
            : parent_(parent)
            , state_(Reorg::State::normal)
            , counter_(-1)
            , slaves_(alloc)
            , actions_(alloc)
            , acks_(alloc)
            , params_(std::nullopt)
        {
        }
    };

    const Log& log_;
    allocator_type alloc_;
    libguarded::plain_guarded<Data> data_;

    auto acknowledge(
        Reorg::State expected,
        AccountsJobs work,
        std::string_view action,
        SlaveID id) noexcept -> void;
    auto acknowledge(
        Data& data,
        Reorg::State expected,
        AccountsJobs work,
        std::string_view action,
        SlaveID id) noexcept -> void;
    auto check_condition(
        const Data& data,
        AccountsJobs work,
        std::string_view action) noexcept -> bool;
    auto check_prepare_reorg(const Data& data) noexcept -> bool;
    auto check_shutdown(const Data& data) noexcept -> bool;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::blockchain::node::wallet
