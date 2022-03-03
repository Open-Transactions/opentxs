// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/wallet/Proposal.hpp"  // IWYU pragma: associated

#include <mutex>
#include <stdexcept>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainTransactionProposal.pb.h"
#include "util/LMDB.hpp"

namespace opentxs::blockchain::database::wallet
{
using Direction = storage::lmdb::LMDB::Dir;
constexpr auto table_{Table::Proposals};

struct Proposal::Imp {
    auto CompletedProposals() const noexcept -> UnallocatedSet<OTIdentifier>
    {
        auto lock = Lock{lock_};

        return finished_proposals_;
    }
    auto Exists(const Identifier& id) const noexcept -> bool
    {
        return lmdb_.Exists(table_, id.Bytes());
    }
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>
    {
        return load_proposal(id);
    }
    auto LoadProposals() const noexcept
        -> UnallocatedVector<proto::BlockchainTransactionProposal>
    {
        auto output = UnallocatedVector<proto::BlockchainTransactionProposal>{};
        lmdb_.Read(
            table_,
            [&](const auto key, const auto value) -> bool {
                output.emplace_back(
                    proto::Factory<proto::BlockchainTransactionProposal>(
                        value.data(), value.size()));

                return true;
            },
            Direction::Forward);

        return output;
    }

    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool
    {
        try {
            const auto bytes = [&] {
                auto out = Space{};

                if (false == proto::write(tx, writer(out))) {
                    throw std::runtime_error{"failed to serialize proposal"};
                }

                return out;
            }();

            if (lmdb_.Store(table_, id.Bytes(), reader(bytes)).first) {
                LogVerbose()(OT_PRETTY_CLASS())("proposal ")(id)(" added ")
                    .Flush();

                return true;
            } else {
                throw std::runtime_error{"failed to store proposal"};
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    auto CancelProposal(MDB_txn* tx, const Identifier& id) noexcept -> bool
    {
        if (lmdb_.Delete(table_, id.Bytes(), tx)) {
            LogVerbose()(OT_PRETTY_CLASS())("proposal ")(id)(" cancelled ")
                .Flush();

            return true;
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to cancel proposal ")(id)
                .Flush();

            return false;
        }
    }
    auto FinishProposal(MDB_txn* tx, const Identifier& id) noexcept -> bool
    {
        const auto out = CancelProposal(tx, id);
        auto lock = Lock{lock_};
        finished_proposals_.emplace(id);

        return out;
    }
    auto ForgetProposals(const UnallocatedSet<OTIdentifier>& ids) noexcept
        -> bool
    {
        auto lock = Lock{lock_};

        for (const auto& id : ids) { finished_proposals_.erase(id); }

        return true;
    }

    Imp(const storage::lmdb::LMDB& lmdb) noexcept
        : lmdb_(lmdb)
        , lock_()
        , finished_proposals_()
    {
    }

private:
    using FinishedProposals = UnallocatedSet<OTIdentifier>;

    const storage::lmdb::LMDB& lmdb_;
    mutable std::mutex lock_;
    mutable FinishedProposals finished_proposals_;

    auto load_proposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>
    {
        auto out = std::optional<proto::BlockchainTransactionProposal>{};
        lmdb_.Load(Table::Proposals, id.Bytes(), [&](const auto bytes) {
            out = proto::Factory<proto::BlockchainTransactionProposal>(
                bytes.data(), bytes.size());
        });

        return out;
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Proposal::Proposal(const storage::lmdb::LMDB& lmdb) noexcept
    : imp_(std::make_unique<Imp>(lmdb))
{
    OT_ASSERT(imp_);
}

auto Proposal::AddProposal(
    const Identifier& id,
    const proto::BlockchainTransactionProposal& tx) noexcept -> bool
{
    return imp_->AddProposal(id, tx);
}

auto Proposal::CancelProposal(MDB_txn* tx, const Identifier& id) noexcept
    -> bool
{
    return imp_->CancelProposal(tx, id);
}

auto Proposal::CompletedProposals() const noexcept
    -> UnallocatedSet<OTIdentifier>
{
    return imp_->CompletedProposals();
}

auto Proposal::Exists(const Identifier& id) const noexcept -> bool
{
    return imp_->Exists(id);
}

auto Proposal::FinishProposal(MDB_txn* tx, const Identifier& id) noexcept
    -> bool
{
    return imp_->FinishProposal(tx, id);
}

auto Proposal::ForgetProposals(const UnallocatedSet<OTIdentifier>& ids) noexcept
    -> bool
{
    return imp_->ForgetProposals(ids);
}

auto Proposal::LoadProposal(const Identifier& id) const noexcept
    -> std::optional<proto::BlockchainTransactionProposal>
{
    return imp_->LoadProposal(id);
}

auto Proposal::LoadProposals() const noexcept
    -> UnallocatedVector<proto::BlockchainTransactionProposal>
{
    return imp_->LoadProposals();
}

Proposal::~Proposal() = default;
}  // namespace opentxs::blockchain::database::wallet
