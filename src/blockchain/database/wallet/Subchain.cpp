// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain

#include "blockchain/database/wallet/Subchain.hpp"  // IWYU pragma: associated

#include "blockchain/database/wallet/SubchainPrivate.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/Position.hpp"

namespace opentxs::blockchain::database::wallet
{
SubchainData::SubchainData(
    const api::Session& api,
    const storage::lmdb::Database& lmdb,
    const blockchain::cfilter::Type filter) noexcept
    : imp_(std::make_unique<SubchainPrivate>(api, lmdb, filter))
{
    OT_ASSERT(imp_);
}

auto SubchainData::GetSubchainID(
    const SubaccountID& subaccount,
    const crypto::Subchain subchain) const noexcept -> SubchainID
{
    return imp_->GetID(subaccount, subchain);
}

auto SubchainData::GetSubchainID(
    const SubaccountID& subaccount,
    const crypto::Subchain subchain,
    storage::lmdb::Transaction& tx) const noexcept -> SubchainID
{
    return imp_->GetID(subaccount, subchain, tx);
}

auto SubchainData::GetPatterns(const SubchainID& subchain, alloc::Default alloc)
    const noexcept -> Patterns
{
    return imp_->GetPatterns(subchain, alloc);
}

auto SubchainData::Reorg(
    const node::internal::HeaderOraclePrivate& data,
    storage::lmdb::Transaction& tx,
    const node::HeaderOracle& headers,
    const SubchainID& subchain,
    const block::Height lastGoodHeight) const noexcept(false) -> bool
{
    return imp_->Reorg(data, headers, subchain, lastGoodHeight, tx);
}

auto SubchainData::SubchainAddElements(
    const SubchainID& subchain,
    const ElementMap& elements) const noexcept -> bool
{
    return imp_->AddElements(subchain, elements);
}

auto SubchainData::SubchainLastIndexed(
    const SubchainID& subchain) const noexcept -> std::optional<Bip32Index>
{
    return imp_->GetLastIndexed(subchain);
}

auto SubchainData::SubchainLastScanned(
    const SubchainID& subchain) const noexcept -> block::Position
{
    return imp_->GetLastScanned(subchain);
}

auto SubchainData::SubchainSetLastScanned(
    const SubchainID& subchain,
    const block::Position& position) const noexcept -> bool
{
    return imp_->SetLastScanned(subchain, position);
}

SubchainData::~SubchainData() = default;
}  // namespace opentxs::blockchain::database::wallet
