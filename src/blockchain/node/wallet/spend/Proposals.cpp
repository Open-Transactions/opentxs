// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/spend/Proposals.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionProposal.pb.h>
#include <chrono>
#include <compare>
#include <functional>
#include <mutex>
#include <optional>
#include <utility>

#include "blockchain/node/wallet/spend/BitcoinTransactionBuilder.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Category.hpp"        // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/SendResult.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
struct Proposals::Imp {
public:
    auto Add(const Proposal& tx, std::promise<SendOutcome>&& promise)
        const noexcept -> void
    {
        auto lock = Lock{lock_};
        auto id = api_.Factory().IdentifierFromBase58(tx.id());

        if (false == db_.AddProposal(id, tx)) {
            LogError()(OT_PRETTY_CLASS())("Database error").Flush();
            promise.set_value(
                {SendResult::DatabaseError, block::TransactionHash{}});
        }

        pending_.Add(std::move(id), std::move(promise));
    }
    auto Run() noexcept -> bool
    {
        auto lock = Lock{lock_};
        send(lock);
        rebroadcast(lock);
        cleanup(lock);

        return pending_.HasData();
    }

    Imp(const api::Session& api,
        const node::Manager& node,
        database::Wallet& db,
        const Type chain) noexcept
        : api_(api)
        , node_(node)
        , db_(db)
        , chain_(chain)
        , lock_()
        , pending_()
        , confirming_()
    {
        for (const auto& serialized : db_.LoadProposals()) {
            auto id = api_.Factory().IdentifierFromBase58(serialized.id());

            if (serialized.has_finished()) {
                confirming_.emplace(std::move(id), Time{});
            } else {
                pending_.Add(std::move(id), {});
            }
        }
    }

private:
    using Builder = std::function<BuildResult(
        const identifier::Generic& id,
        Proposal&,
        std::promise<SendOutcome>&)>;
    using Promise = std::promise<SendOutcome>;
    using Data = std::pair<identifier::Generic, Promise>;

    struct Pending {
        auto Exists(const identifier::Generic& id) const noexcept -> bool
        {
            auto lock = Lock{lock_};

            return 0 < ids_.count(id);
        }
        auto HasData() const noexcept -> bool
        {
            auto lock = Lock{lock_};

            return 0 < data_.size();
        }

        auto Add(
            identifier::Generic&& id,
            std::promise<SendOutcome>&& promise) noexcept -> void
        {
            auto lock = Lock{lock_};

            if (0 < ids_.count(id)) {
                LogError()(OT_PRETTY_CLASS())("Proposal already exists")
                    .Flush();
                promise.set_value(
                    {SendResult::DuplicateProposal, block::TransactionHash{}});
            }

            ids_.emplace(id);
            data_.emplace_back(std::move(id), std::move(promise));
        }
        auto Add(Data&& job) noexcept -> void
        {
            auto lock = Lock{lock_};
            const auto& [id, promise] = job;

            OT_ASSERT(0 == ids_.count(id));

            ids_.emplace(id);
            data_.emplace_back(std::move(job));
        }
        auto Delete(const identifier::Generic& id) noexcept -> void
        {
            auto lock = Lock{lock_};
            auto copy = identifier::Generic{id};

            if (0 < ids_.count(copy)) {
                ids_.erase(copy);

                for (auto i{data_.begin()}; i != data_.end();) {
                    if (i->first == copy) {
                        i = data_.erase(i);
                    } else {
                        ++i;
                    }
                }
            }
        }
        auto Pop() noexcept -> Data
        {
            auto lock = Lock{lock_};
            auto post = ScopeGuard{[&] { data_.pop_front(); }};

            return std::move(data_.front());
        }

        Pending() noexcept
            : lock_()
            , data_()
            , ids_()
        {
        }

    private:
        mutable std::mutex lock_;
        UnallocatedDeque<Data> data_;
        UnallocatedSet<identifier::Generic> ids_;
    };

    const api::Session& api_;
    const node::Manager& node_;
    database::Wallet& db_;
    const Type chain_;
    mutable std::mutex lock_;
    mutable Pending pending_;
    mutable UnallocatedMap<identifier::Generic, Time> confirming_;

    static auto is_expired(const Proposal& tx) noexcept -> bool
    {
        return Clock::now() > convert_stime(tx.expires());
    }

    auto build_transaction_bitcoin(
        const identifier::Generic& id,
        Proposal& proposal,
        std::promise<SendOutcome>& promise) const noexcept -> BuildResult
    {
        using TxBuilder = BitcoinTransactionBuilder;

        return TxBuilder{api_, node_, id, chain_, db_, proposal, promise}();
    }
    auto cleanup(const Lock& lock) noexcept -> void
    {
        const auto finished = db_.CompletedProposals();

        for (const auto& id : finished) {
            pending_.Delete(id);
            confirming_.erase(id);
        }

        db_.ForgetProposals(finished);
    }
    auto get_builder() const noexcept -> Builder
    {
        using enum opentxs::blockchain::Category;

        switch (category(chain_)) {
            case output_based: {

                return [this](const auto& id, auto& in, auto& out) -> auto {
                    return build_transaction_bitcoin(id, in, out);
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
    auto rebroadcast(const Lock& lock) noexcept -> void
    {
        // TODO monitor inv messages from peers and wait until a peer
        // we didn't transmit the transaction to tells us about it
        constexpr auto limit = std::chrono::minutes(1);

        for (auto& [id, time] : confirming_) {
            if ((Clock::now() - time) < limit) { continue; }

            auto proposal = db_.LoadProposal(id);

            if (false == proposal.has_value()) { continue; }

            auto tx = api_.Factory().InternalSession().BlockchainTransaction(
                proposal.value().finished(), {});

            if (false == tx.IsValid()) { continue; }

            node_.Internal().BroadcastTransaction(tx, true);
        }
    }
    auto send(const Lock& lock) noexcept -> void
    {
        if (false == pending_.HasData()) { return; }

        auto job = pending_.Pop();
        auto wipe{false};
        auto erase{false};
        auto loop = ScopeGuard{[&]() {
            const auto& [id, promise] = job;

            if (wipe) {
                db_.CancelProposal(id);
                erase = true;
            }

            if (false == erase) { pending_.Add(std::move(job)); }
        }};
        auto& [id, promise] = job;
        auto serialized = db_.LoadProposal(id);

        if (false == serialized.has_value()) { return; }

        auto& data = *serialized;

        if (is_expired(data)) {
            wipe = true;

            return;
        }

        if (auto builder = get_builder(); builder) {
            switch (builder(id, data, promise)) {
                case BuildResult::PermanentFailure: {
                    wipe = true;
                    [[fallthrough]];
                }
                case BuildResult::TemporaryFailure: {

                    return;
                }
                case BuildResult::Success:
                default: {
                    confirming_.emplace(std::move(id), Clock::now());
                    erase = true;
                }
            }
        }
    }
};

Proposals::Proposals(
    const api::Session& api,
    const node::Manager& node,
    database::Wallet& db,
    const Type chain) noexcept
    : imp_(std::make_unique<Imp>(api, node, db, chain))
{
    OT_ASSERT(imp_);
}

auto Proposals::Add(const Proposal& tx, std::promise<SendOutcome>&& promise)
    const noexcept -> void
{
    imp_->Add(tx, std::move(promise));
}

auto Proposals::Run() noexcept -> bool { return imp_->Run(); }

Proposals::~Proposals() = default;
}  // namespace opentxs::blockchain::node::wallet
