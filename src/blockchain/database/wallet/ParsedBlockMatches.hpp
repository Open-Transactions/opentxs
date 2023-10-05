// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>
#include <utility>

#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/wallet/Types.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Transaction;
}  // namespace block

namespace database
{
namespace wallet
{
class OutputCache;
struct Modification;
}  // namespace wallet
}  // namespace database

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet
{
struct ParsedBlockMatches final : public pmr::Allocated {
    const block::Height target_;
    FlatMap<block::Outpoint, Modification> transaction_;
    FlatSet<identifier::Generic> proposals_;

    [[nodiscard]] static auto Parse(
        const api::Session& api,
        const Log& log,
        const OutputCache& cache,
        const block::Position& block,
        BlockMatches& matches,
        Set<block::Transaction>& processed,
        TXOs& txoCreated,
        ConsumedTXOs& txoConsumed,
        bool mempool,
        block::Height target,
        alloc::Strategy alloc) noexcept(false) -> ParsedBlockMatches;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    ParsedBlockMatches() = delete;
    ParsedBlockMatches(block::Height target, allocator_type alloc) noexcept;
    ParsedBlockMatches(ParsedBlockMatches&& rhs) noexcept;
    ParsedBlockMatches(ParsedBlockMatches&& rhs, allocator_type alloc) noexcept;
    ParsedBlockMatches(const ParsedBlockMatches&) = delete;

private:
    enum class Transition {
        Null,
        Allowed,
        Disallowed,
        Unnecessary,
    };

    static auto associate_input(
        const std::size_t index,
        const protocol::bitcoin::base::block::Output& output,
        protocol::bitcoin::base::block::internal::Transaction&
            tx) noexcept(false) -> void;
    static auto check_transition(
        node::TxoState initialState,
        node::TxoState finalState) noexcept(false) -> Transition;
    static auto initial_state(
        const Log& log,
        const block::Outpoint& id,
        const OutputCache& cache,
        node::TxoState finalState) noexcept(false)
        -> std::pair<std::optional<node::TxoState>, Transition>;

    auto find_output(
        const block::Outpoint& id,
        const OutputCache& cache,
        const BlockMatches& matches) const noexcept(false)
        -> const protocol::bitcoin::base::block::Output&;
    auto scan_transactions(
        const Log& log,
        const block::Position& block,
        ParsedTXOs& created,
        ParsedTXOs& consumed,
        Set<block::Outpoint>& generation,
        BlockMatches& matches,
        Set<block::Transaction>& processed) const noexcept(false) -> void;

    auto get_proposals(
        const block::Outpoint& id,
        const OutputCache& cache,
        bool mempool,
        Set<identifier::Generic>& proposals,
        alloc::Strategy alloc) noexcept(false) -> void;
    auto parse_consumed(
        const Log& log,
        const OutputCache& cache,
        const BlockMatches& matches,
        bool mempool,
        ParsedTXOs& consumed,
        ConsumedTXOs& out,
        alloc::Strategy alloc) noexcept(false) -> void;
    auto parse_created(
        const api::Session& api,
        const Log& log,
        const block::Position& block,
        const Set<block::Outpoint>& generation,
        const OutputCache& cache,
        bool mempool,
        ParsedTXOs& created,
        TXOs& out,
        alloc::Strategy alloc) noexcept(false) -> void;
};
}  // namespace opentxs::blockchain::database::wallet
