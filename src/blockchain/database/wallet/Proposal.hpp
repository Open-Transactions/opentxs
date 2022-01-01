// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <vector>

#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs
{
namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

extern "C" {
typedef struct MDB_txn MDB_txn;
}

namespace opentxs::blockchain::database::wallet
{
class Proposal
{
public:
    auto CompletedProposals() const noexcept -> std::pmr::set<OTIdentifier>;
    auto Exists(const Identifier& id) const noexcept -> bool;
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>;
    auto LoadProposals() const noexcept
        -> std::pmr::vector<proto::BlockchainTransactionProposal>;

    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool;
    auto CancelProposal(MDB_txn* tx, const Identifier& id) noexcept -> bool;
    auto FinishProposal(MDB_txn* tx, const Identifier& id) noexcept -> bool;
    auto ForgetProposals(const std::pmr::set<OTIdentifier>& ids) noexcept
        -> bool;

    Proposal(const storage::lmdb::LMDB& lmdb) noexcept;

    ~Proposal();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Proposal() = delete;
    Proposal(const Proposal&) = delete;
    auto operator=(const Proposal&) -> Proposal& = delete;
    auto operator=(Proposal&&) -> Proposal& = delete;
};
}  // namespace opentxs::blockchain::database::wallet
