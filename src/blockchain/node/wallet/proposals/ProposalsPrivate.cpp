// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/proposals/ProposalsPrivate.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionProposal.pb.h>
#include <chrono>
#include <compare>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "blockchain/node/spend/Imp.hpp"
#include "blockchain/node/wallet/proposals/BitcoinTransactionBuilder.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Spend.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Category.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
ProposalsPrivate::ProposalsPrivate(
    const api::session::Client& api,
    const node::Manager& node,
    database::Wallet& db,
    const Type chain) noexcept
    : api_(api)
    , node_(node)
    , chain_(chain)
    , data_(db)
{
    auto handle = data_.lock();
    auto& data = *handle;

    for (const auto& serialized : data.db_.LoadProposals()) {
        auto id = api_.Factory().IdentifierFromBase58(serialized.id());

        if (serialized.has_finished()) {
            data.confirming_.emplace(std::move(id), Time{});
        } else {
            data.pending_.Add(std::move(id), {});
        }
    }
}

auto ProposalsPrivate::Add(
    const node::Spend& spend,
    std::promise<SendOutcome>&& promise) const noexcept -> bool
{
    const auto& log = LogTrace();
    auto alloc = alloc::Strategy{};  // TODO

    try {
        static const auto blank = block::TransactionHash{};
        const auto& id = spend.ID();
        using enum SendResult;
        const auto proto = [&] {
            auto out = proto::BlockchainTransactionProposal{};

            if (false == spend.Internal().Serialize(out)) {
                promise.set_value({SerializationError, blank});

                throw std::runtime_error{"failed to serialize spend"};
            }

            return out;
        }();
        auto handle = data_.lock();
        auto& data = *handle;

        if (false == data.db_.AddProposal(log, id, proto, alloc)) {
            promise.set_value({DatabaseError, blank});

            throw std::runtime_error{"failed to save spend"};
        }

        return data.pending_.Add(id, std::move(promise));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto ProposalsPrivate::build_transaction_bitcoin(
    Data& data,
    const identifier::Generic& id,
    node::Spend& spend,
    std::promise<SendOutcome>& promise) const noexcept -> BuildResult
{
    using TxBuilder = BitcoinTransactionBuilder;

    return TxBuilder{api_, node_, id, chain_, data.db_, spend, promise}();
}

auto ProposalsPrivate::cleanup(Data& data) noexcept -> void
{
    const auto& log = LogTrace();
    auto alloc = alloc::Strategy{};  // TODO
    const auto finished = data.db_.CompletedProposals();

    for (const auto& id : finished) {
        data.pending_.Delete(id);
        data.confirming_.erase(id);
    }

    data.db_.ForgetProposals(log, finished, alloc);
}

auto ProposalsPrivate::get_builder(Data& data) const noexcept -> Builder
{
    using enum opentxs::blockchain::Category;

    switch (category(chain_)) {
        case output_based: {

            return [&, this](const auto& id, auto& in, auto& out) -> auto {
                return build_transaction_bitcoin(data, id, in, out);
            };
        }
        case unknown_category:
        case balance_based:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported chain").Flush();

            return {};
        }
    }
}

auto ProposalsPrivate::rebroadcast(Data& data) noexcept -> void
{
    // TODO monitor inv messages from peers and wait until a peer
    // we didn't transmit the transaction to tells us about it
    constexpr auto limit = std::chrono::minutes(1);

    for (auto& [id, time] : data.confirming_) {
        if ((Clock::now() - time) < limit) { continue; }

        auto proposal = data.db_.LoadProposal(id);

        if (false == proposal.has_value()) { continue; }

        auto tx = api_.Factory().InternalSession().BlockchainTransaction(
            proposal.value().finished(), {});

        if (false == tx.IsValid()) { continue; }

        node_.Internal().BroadcastTransaction(tx, true);
    }
}

auto ProposalsPrivate::Run() noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;
    send(data);
    rebroadcast(data);
    cleanup(data);

    return data.pending_.HasData();
}

auto ProposalsPrivate::send(Data& data) noexcept -> void
{
    if (false == data.pending_.HasData()) { return; }

    const auto& log = LogTrace();
    auto alloc = alloc::Strategy{};  // TODO
    auto job = data.pending_.Pop();
    auto wipe{false};
    auto erase{false};
    auto loop = ScopeGuard{[&]() {
        const auto& [id, promise] = job;

        if (wipe) {
            data.db_.CancelProposal(log, id, alloc);
            erase = true;
        }

        if (false == erase) { data.pending_.Add(std::move(job)); }
    }};
    auto& [id, promise] = job;
    auto serialized = data.db_.LoadProposal(id);

    if (false == serialized.has_value()) { return; }

    try {
        auto& proto = *serialized;
        auto spend = node::Spend{
            std::make_unique<SpendPrivate>(api_, chain_, proto).release()};
        // TODO check to see if a transaction was already created. If so,
        // rebroadcast it.

        if (spend.Internal().IsExpired()) {
            wipe = true;

            return;
        }

        if (auto builder = get_builder(data); builder) {
            switch (builder(id, spend, promise)) {
                case BuildResult::PermanentFailure: {
                    wipe = true;
                    [[fallthrough]];
                }
                case BuildResult::TemporaryFailure: {

                    return;
                }
                case BuildResult::Success:
                default: {
                    data.confirming_.emplace(std::move(id), Clock::now());
                    erase = true;
                }
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return;
    }
}
}  // namespace opentxs::blockchain::node::wallet
