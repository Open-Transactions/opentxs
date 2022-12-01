// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "network/blockchain/peer/UpdateBlockJob.hpp"  // IWYU pragma: associated

#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"

namespace opentxs::network::blockchain::internal
{
Peer::Imp::UpdateBlockJob::UpdateBlockJob(ReadView data) noexcept
    : data_(data)
{
}

auto Peer::Imp::UpdateBlockJob::operator()(std::monostate& job) noexcept
    -> JobUpdate
{
    return {false, false};
}

auto Peer::Imp::UpdateBlockJob::operator()(
    opentxs::blockchain::node::internal::HeaderJob& job) noexcept -> JobUpdate
{
    return {false, false};
}

auto Peer::Imp::UpdateBlockJob::operator()(
    opentxs::blockchain::node::internal::BlockBatch& job) noexcept -> JobUpdate
{
    return {true, job.Submit(data_)};
}
}  // namespace opentxs::network::blockchain::internal
