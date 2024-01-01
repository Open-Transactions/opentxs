// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <optional>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

namespace protobuf
{
class BlockchainTransactionProposal;
}  // namespace protobuf

namespace storage
{
namespace lmdb
{
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet
{
class Proposal
{
public:
    auto CompletedProposals() const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto Exists(const identifier::Generic& id) const noexcept -> bool;
    auto LoadProposal(const identifier::Generic& id) const noexcept
        -> std::optional<protobuf::BlockchainTransactionProposal>;
    auto LoadProposals() const noexcept
        -> UnallocatedVector<protobuf::BlockchainTransactionProposal>;

    auto AddProposal(
        const identifier::Generic& id,
        const protobuf::BlockchainTransactionProposal& tx) noexcept -> bool;
    auto CancelProposal(
        storage::lmdb::Transaction& tx,
        const identifier::Generic& id) noexcept -> bool;
    auto FinishProposal(
        storage::lmdb::Transaction& tx,
        const identifier::Generic& id) noexcept -> bool;
    auto ForgetProposals(
        const UnallocatedSet<identifier::Generic>& ids) noexcept -> bool;

    Proposal(
        const api::Crypto& crypto,
        const storage::lmdb::Database& lmdb) noexcept;
    Proposal() = delete;
    Proposal(const Proposal&) = delete;
    auto operator=(const Proposal&) -> Proposal& = delete;
    auto operator=(Proposal&&) -> Proposal& = delete;

    ~Proposal();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::wallet
