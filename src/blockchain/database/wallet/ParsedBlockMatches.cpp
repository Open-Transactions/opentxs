// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/wallet/ParsedBlockMatches.hpp"  // IWYU pragma: associated

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <functional>
#include <span>
#include <stdexcept>
#include <tuple>

#include "blockchain/database/wallet/Modification.hpp"
#include "blockchain/database/wallet/OutputCache.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch-enum"
namespace opentxs::blockchain::database::wallet
{
using namespace std::literals;
using enum node::TxoState;

ParsedBlockMatches::ParsedBlockMatches(
    block::Height target,
    allocator_type alloc) noexcept
    : Allocated({alloc})
    , target_(target)
    , transaction_(alloc)
    , proposals_(alloc)
{
}

ParsedBlockMatches::ParsedBlockMatches(ParsedBlockMatches&& rhs) noexcept
    : ParsedBlockMatches(std::move(rhs), rhs.get_allocator())
{
}

ParsedBlockMatches::ParsedBlockMatches(
    ParsedBlockMatches&& rhs,
    allocator_type alloc) noexcept
    : Allocated({alloc})
    , target_(rhs.target_)
    , transaction_(std::move(rhs.transaction_), alloc)
    , proposals_(std::move(rhs.proposals_), alloc)
{
}

auto ParsedBlockMatches::associate_input(
    const std::size_t index,
    const protocol::bitcoin::base::block::Output& output,
    protocol::bitcoin::base::block::internal::Transaction& tx) noexcept(false)
    -> void
{
    if (false == tx.AssociatePreviousOutput(index, output)) {
        throw std::runtime_error{"error associating previous output to input"};
    }
}

auto ParsedBlockMatches::check_transition(
    node::TxoState initialState,
    node::TxoState finalState) noexcept(false) -> Transition
{
    // NOTE not all final states are supported by this function, only the final
    // states applicable to the AddConfirmedTransactions and
    // AddMempoolTransaction functions.

    // NOTE support for the OrphanedNew and OrphanedSpend is incomplete.

    using enum Transition;

    switch (finalState) {
        case UnconfirmedNew: {
            switch (initialState) {
                case UnconfirmedSpend:
                case UnconfirmedNew: {

                    return Unnecessary;
                }
                case ConfirmedNew:
                case ConfirmedSpend:
                case OrphanedNew:
                case OrphanedSpend:
                case Immature: {

                    return Disallowed;
                }
                case Error:
                case All:
                default: {
                    throw std::runtime_error{
                        "invalid initial state: "s.append(print(finalState))};
                }
            }
        }
        case UnconfirmedSpend: {
            switch (initialState) {
                case UnconfirmedNew:
                case ConfirmedNew:
                case Immature: {

                    return Allowed;
                }
                case UnconfirmedSpend: {

                    return Unnecessary;
                }
                case ConfirmedSpend:
                case OrphanedNew:
                case OrphanedSpend: {

                    return Disallowed;
                }
                case Error:
                case All:
                default: {
                    throw std::runtime_error{
                        "invalid initial state: "s.append(print(finalState))};
                }
            }
        }
        case ConfirmedNew: {
            switch (initialState) {
                case UnconfirmedNew: {

                    return Allowed;
                }
                // NOTE since not all blockchains have the same rules about
                // transaction ordering within a block, it's possible in some
                // cases that we will end up processing a transaction which
                // spends an output before we process the transaction which
                // creates the output. Since spends always happen after creation
                // we must skip the outputs in these cases to commit the most
                // recent state to the database.
                case UnconfirmedSpend:
                case ConfirmedSpend:
                case ConfirmedNew: {

                    return Unnecessary;
                }
                case OrphanedNew:
                case OrphanedSpend:
                case Immature: {

                    return Disallowed;
                }
                case Error:
                case All:
                default: {
                    throw std::runtime_error{
                        "invalid initial state: "s.append(print(finalState))};
                }
            }
        }
        case ConfirmedSpend: {
            switch (initialState) {
                case UnconfirmedNew:
                case ConfirmedNew:
                case UnconfirmedSpend:
                case Immature: {

                    return Allowed;
                }
                case ConfirmedSpend: {

                    return Unnecessary;
                }
                case OrphanedNew:
                case OrphanedSpend: {

                    return Disallowed;
                }
                case Error:
                case All:
                default: {
                    throw std::runtime_error{
                        "invalid initial state: "s.append(print(finalState))};
                }
            }
        }
        case Immature: {
            switch (initialState) {
                case OrphanedNew: {

                    return Allowed;
                }
                case Immature: {

                    return Unnecessary;
                }
                case UnconfirmedNew:
                case UnconfirmedSpend:
                case ConfirmedNew:
                case ConfirmedSpend:
                case OrphanedSpend: {

                    return Disallowed;
                }
                case Error:
                case All:
                default: {
                    throw std::runtime_error{
                        "invalid initial state: "s.append(print(finalState))};
                }
            }
        }
        case Error:
        case OrphanedNew:
        case OrphanedSpend:
        case All:
        default: {
            throw std::runtime_error{
                "invalid final state: "s.append(print(finalState))};
        }
    }
}

auto ParsedBlockMatches::find_output(
    const block::Outpoint& id,
    const OutputCache& cache,
    const BlockMatches& matches) const noexcept(false)
    -> const protocol::bitcoin::base::block::Output&
{
    const auto& first = transaction_;
    const auto& second = matches;

    if (cache.Exists(id)) {

        return cache.GetOutput(id);
    } else if (const auto i = first.find(id); first.end() != i) {
        const auto& opt = i->second.output_;

        assert_true(opt.has_value());

        return *opt;
    } else if (const auto j = second.find(id.Txid()); second.end() != j) {
        const auto txout = std::get<2>(j->second).asBitcoin().Outputs();
        const auto n = id.Index();

        if (n >= txout.size()) {
            throw std::runtime_error{"transaction "s.append(j->first.asHex())
                                         .append(" does not contain output ")
                                         .append(std::to_string(n))};
        }

        return txout[n];
    } else {
        throw std::runtime_error{"unable to locate output "s.append(id.str())};
    }
}

auto ParsedBlockMatches::get_proposals(
    const block::Outpoint& id,
    const OutputCache& cache,
    bool mempool,
    Set<identifier::Generic>& proposals,
    alloc::Strategy alloc) noexcept(false) -> void
{
    if (false == mempool) {
        for (auto& proposal : cache.GetProposals(id, alloc)) {
            proposals_.emplace(proposal);
            proposals.emplace(std::move(proposal));
        }
    }
}

auto ParsedBlockMatches::initial_state(
    const Log& log,
    const block::Outpoint& id,
    const OutputCache& cache,
    node::TxoState finalState) noexcept(false)
    -> std::pair<std::optional<node::TxoState>, Transition>
{
    using enum Transition;

    if (cache.Exists(id)) {
        const auto& output = cache.GetOutput(id).Internal();
        const auto state = output.State();

        return std::make_pair(state, check_transition(state, finalState));
    } else {

        return std::make_pair(std::nullopt, Null);
    }
}

auto ParsedBlockMatches::Parse(
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
    alloc::Strategy alloc) noexcept(false) -> ParsedBlockMatches
{
    auto created = ParsedTXOs{alloc.work_};
    auto consumed = ParsedTXOs{alloc.work_};
    auto gen = Set<block::Outpoint>{alloc.work_};
    auto out = ParsedBlockMatches{target, alloc.result_};
    out.scan_transactions(
        log, block, created, consumed, gen, matches, processed);

    if (mempool && (false == gen.empty())) {
        throw std::runtime_error{"one or more mempool transactions claims to "
                                 "contain generation outputs"};
    }

    out.parse_created(
        api, log, block, gen, cache, mempool, created, txoCreated, alloc);
    out.parse_consumed(
        log, cache, matches, mempool, consumed, txoConsumed, alloc);

    for (auto& item : matches) {
        auto& [txid, match] = item;
        auto& [inputs, _, txn] = match;
        auto& tx = txn.asBitcoin();
        const auto txin = tx.Inputs();

        for (auto index : inputs) {
            const auto& input = txin[index];
            const auto& outpoint = input.PreviousOutput();
            const auto& spends = out.find_output(outpoint, cache, matches);
            associate_input(index, spends, tx.Internal().asBitcoin());
        }
    }

    return out;
}

auto ParsedBlockMatches::parse_consumed(
    const Log& log,
    const OutputCache& cache,
    const BlockMatches& matches,
    bool mempool,
    ParsedTXOs& consumed,
    ConsumedTXOs& out,
    alloc::Strategy alloc) noexcept(false) -> void
{
    using enum Transition;

    log()("parsing ")(consumed.size())(" consumed outputs").Flush();

    for (auto& [outpoint, data] : consumed) {
        const auto finalState = [&] {
            if (mempool) {

                return UnconfirmedSpend;
            } else {

                return ConfirmedSpend;
            }
        }();

        if (auto i = transaction_.find(outpoint); transaction_.end() != i) {
            auto& params = i->second;

            assert_true(params.output_.has_value());
            assert_true(params.output_->IsValid());

            if (false == mempool) { out.emplace(outpoint); }

            // NOTE output creation always happens logically before output
            // consumption, therefore the final state when the output was
            // created is the initial state when then output is consumed.
            const auto& initialState = params.final_state_;

            switch (check_transition(initialState, finalState)) {
                case Disallowed: {
                    throw std::runtime_error{
                        "transition for output "s.append(outpoint.str())
                            .append(" from state ")
                            .append(print(initialState))
                            .append(" to state ")
                            .append(print(finalState))
                            .append(" is not allowed")};
                }
                case Unnecessary: {
                    log()("skipping transition for output ")(
                        outpoint)(" from state ")(print(initialState))(
                        " to state ")(print(finalState))
                        .Flush();
                    continue;
                }
                case Allowed: {
                    log()("transitioning output ")(outpoint)(" from state ")(
                        print(initialState))(" to state ")(print(finalState))
                        .Flush();
                } break;
                case Null:
                default: {
                    LogAbort()()("invalid transition").Abort();
                }
            }

            params.final_state_ = finalState;
            get_proposals(outpoint, cache, mempool, params.proposals_, alloc);
        } else {
            const auto [initialState, transition] =
                initial_state(log, outpoint, cache, finalState);

            switch (transition) {
                case Disallowed: {
                    assert_true(initialState.has_value());

                    throw std::runtime_error{
                        "transition for output "s.append(outpoint.str())
                            .append(" from state ")
                            .append(print(*initialState))
                            .append(" to state ")
                            .append(print(finalState))
                            .append(" is not allowed")};
                }
                case Unnecessary: {
                    assert_true(initialState.has_value());

                    log()("skipping transition for output ")(
                        outpoint)(" from state ")(print(*initialState))(
                        " to state ")(print(finalState))
                        .Flush();
                    continue;
                }
                case Allowed: {
                    assert_true(initialState.has_value());

                    log()("transitioning output ")(outpoint)(" from state ")(
                        print(*initialState))(" to state ")(print(finalState))
                        .Flush();
                } break;
                case Null: {
                    assert_false(initialState.has_value());

                    throw std::runtime_error{
                        "unable to find previous output for consumed for output "s
                            .append(outpoint.str())};
                }
                default: {
                    LogAbort()()("invalid transition").Abort();
                }
            }

            auto [j, added] = transaction_.try_emplace(outpoint, finalState);

            assert_true(added);

            const auto& spends = find_output(outpoint, cache, matches);

            auto& params = j->second;
            params.final_state_ = finalState;
            params.initial_state_ = initialState;
            params.output_.emplace(spends);

            assert_true(params.output_.has_value());
            assert_true(params.output_->IsValid());

            if (false == mempool) { out.emplace(outpoint); }

            get_proposals(outpoint, cache, mempool, params.proposals_, alloc);
        }
    }
}

auto ParsedBlockMatches::parse_created(
    const api::Session& api,
    const Log& log,
    const block::Position& block,
    const Set<block::Outpoint>& generation,
    const OutputCache& cache,
    bool mempool,
    ParsedTXOs& created,
    TXOs& out,
    alloc::Strategy alloc) noexcept(false) -> void
{
    using enum Transition;

    log()("parsing ")(created.size())(" created outputs").Flush();

    for (auto& [outpoint, data] : created) {
        if (transaction_.contains(outpoint)) {
            throw std::runtime_error{
                "duplicate output: "s.append(outpoint.str())};
        }

        const auto finalState = [&]() {
            if (generation.contains(outpoint)) {
                const auto best = cache.GetPosition().Decode(api).height_;

                if (is_mature(block.height_, best, target_)) {

                    return ConfirmedNew;
                } else {

                    return Immature;
                }
            } else if (mempool) {

                return UnconfirmedNew;
            } else {

                return ConfirmedNew;
            }
        }();
        const auto [initialState, transition] =
            initial_state(log, outpoint, cache, finalState);

        switch (transition) {
            case Disallowed: {
                assert_true(initialState.has_value());

                throw std::runtime_error{
                    "transition for output "s.append(outpoint.str())
                        .append(" from state ")
                        .append(print(*initialState))
                        .append(" to state ")
                        .append(print(finalState))
                        .append(" is not allowed")};
            }
            case Unnecessary: {
                assert_true(initialState.has_value());

                log()("skipping transition for output ")(
                    outpoint)(" from state ")(print(*initialState))(
                    " to state ")(print(finalState))
                    .Flush();
                continue;
            }
            case Allowed: {
                assert_true(initialState.has_value());

                log()("transitioning output ")(outpoint)(" from state ")(
                    print(*initialState))(" to state ")(print(finalState))
                    .Flush();
            } break;
            case Null: {
                assert_false(initialState.has_value());

                log()("creating new output ")(outpoint)(" in state ")(
                    print(finalState))
                    .Flush();
            } break;
            default: {
                LogAbort()()("invalid transition").Abort();
            }
        }

        auto [i, added] = transaction_.try_emplace(outpoint, finalState);

        assert_true(added);

        auto& [output, proposals] = data;
        auto& params = i->second;
        params.final_state_ = finalState;
        params.initial_state_ = initialState;
        params.output_.emplace(output);

        assert_true(params.output_.has_value());
        assert_true(params.output_->IsValid());

        out.try_emplace(outpoint, *params.output_);
        get_proposals(outpoint, cache, mempool, params.proposals_, alloc);
    }
}

auto ParsedBlockMatches::scan_transactions(
    const Log& log,
    const block::Position& block,
    ParsedTXOs& created,
    ParsedTXOs& consumed,
    Set<block::Outpoint>& generation,
    BlockMatches& matches,
    Set<block::Transaction>& processed) const noexcept(false) -> void
{
    for (auto& item : matches) {
        const auto& tx = [&]() -> const auto& {
            auto& [txid, match] = item;
            auto& [inputs, outputs, txn] = match;
            log()("processing transaction ")
                .asHex(txid)(" with ")(inputs.size())(" input matches and ")(
                    outputs.size())(" output matches")
                .Flush();
            auto& out = txn.Internal().asBitcoin();
            out.SetMinedPosition(block);
            processed.emplace(txn);

            return out;
        }();
        auto& [txid, match] = item;
        auto& [inputs, outputs, txn] = match;
        const auto txin = tx.Inputs();
        const auto txout = tx.Outputs();

        for (auto index : inputs) {
            const auto& input = txin[index];
            const auto& outpoint = input.PreviousOutput();
            consumed[outpoint];
            log().asHex(txid)(" input ")(index)(" consumes ")(outpoint).Flush();
        }

        for (auto index : outputs) {
            const auto outpoint = block::Outpoint{txid, index};
            auto& [output, _] = created[outpoint];
            output = txout[index];

            if (false == output.IsValid()) {
                throw std::runtime_error{
                    "transaction "s.append(txid.asHex())
                        .append(" contains invalid output at index ")
                        .append(std::to_string(index))};
            }

            if (tx.IsGeneration()) {
                generation.emplace(outpoint);
                log()("found generation output ")(outpoint).Flush();
            } else {
                log()("found regular output ")(outpoint).Flush();
            }
        }
    }
}
}  // namespace opentxs::blockchain::database::wallet
#pragma GCC diagnostic pop
