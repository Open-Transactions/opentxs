// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <string_view>

#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace node
{
namespace internal
{
struct HeaderOraclePrivate;
}  // namespace internal

namespace wallet
{
class ReorgSlave;
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

namespace storage
{
namespace lmdb
{
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Reorg
{
public:
    struct Params {
        const block::Position& position_;
        storage::lmdb::Transaction tx_;

        Params(
            const block::Position& position,
            storage::lmdb::Transaction&& tx) noexcept
            : position_(position)
            , tx_(std::move(tx))
        {
        }
        Params(const Params&) = delete;
        Params(Params&&) = delete;
        auto operator=(const Params&) -> Params& = delete;
        auto operator=(Params&&) -> Params& = delete;
    };
    using Job = std::function<bool(
        const node::HeaderOracle&,
        const node::internal::HeaderOraclePrivate&,
        Params&)>;

    enum class State {
        normal,
        pre_reorg,
        reorg,
        shutdown,
    };

    [[nodiscard]] virtual auto GetSlave(
        const network::zeromq::Pipeline& parent,
        std::string_view name,
        alloc::Strategy alloc) noexcept -> ReorgSlave = 0;

    virtual ~Reorg() = default;

protected:
    Reorg() = default;
};
}  // namespace opentxs::blockchain::node::wallet
