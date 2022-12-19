// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/peer/RunJob.hpp"  // IWYU pragma: associated

namespace opentxs::network::blockchain::internal
{
Peer::Imp::RunJob::RunJob(Imp& parent, allocator_type monotonic) noexcept
    : parent_(parent)
    , monotonic_(monotonic)
{
}

auto Peer::Imp::RunJob::operator()(std::monostate& job) noexcept -> void {}

auto Peer::Imp::RunJob::operator()(
    opentxs::blockchain::node::internal::HeaderJob& job) noexcept -> void
{
    parent_.transmit_request_block_headers(job, monotonic_);
}

auto Peer::Imp::RunJob::operator()(
    opentxs::blockchain::node::internal::BlockBatch& job) noexcept -> void
{
    parent_.transmit_request_blocks(job, monotonic_);
}
}  // namespace opentxs::network::blockchain::internal
