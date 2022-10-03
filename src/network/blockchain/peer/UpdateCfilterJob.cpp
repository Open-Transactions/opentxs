// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "network/blockchain/peer/UpdateCfilterJob.hpp"  // IWYU pragma: associated

#include <utility>

namespace opentxs::network::blockchain::internal
{
Peer::Imp::UpdateCfilterJob::UpdateCfilterJob(
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Position&& block,
    opentxs::blockchain::GCS&& filter) noexcept
    : block_(std::move(block))
    , type_(std::move(type))
    , filter_(std::move(filter))
{
}

auto Peer::Imp::UpdateCfilterJob::operator()(std::monostate& job) noexcept
    -> JobUpdate
{
    return {false, false};
}

auto Peer::Imp::UpdateCfilterJob::operator()(
    opentxs::blockchain::node::internal::HeaderJob& job) noexcept -> JobUpdate
{
    return {false, false};
}

auto Peer::Imp::UpdateCfilterJob::operator()(
    opentxs::blockchain::node::internal::BlockBatch& job) noexcept -> JobUpdate
{
    return {false, false};
}

auto Peer::Imp::UpdateCfilterJob::operator()(
    opentxs::blockchain::node::CfheaderJob& job) noexcept -> JobUpdate
{
    return {false, false};
}

auto Peer::Imp::UpdateCfilterJob::operator()(
    opentxs::blockchain::node::CfilterJob& job) noexcept -> JobUpdate
{
    const auto rc = job.Download(block_, std::move(filter_), type_);

    if (rc && job.isDownloaded()) { return {true, true}; }

    return {true, !rc};
}
}  // namespace opentxs::network::blockchain::internal
