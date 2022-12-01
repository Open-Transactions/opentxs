// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "network/blockchain/peer/RunJob.hpp"  // IWYU pragma: associated

namespace opentxs::network::blockchain::internal
{
Peer::Imp::RunJob::RunJob(Imp& parent) noexcept
    : parent_(parent)
{
}

auto Peer::Imp::RunJob::operator()(std::monostate& job) noexcept -> void {}

auto Peer::Imp::RunJob::operator()(
    opentxs::blockchain::node::internal::HeaderJob& job) noexcept -> void
{
    parent_.transmit_request_block_headers(job);
}

auto Peer::Imp::RunJob::operator()(
    opentxs::blockchain::node::internal::BlockBatch& job) noexcept -> void
{
    parent_.transmit_request_blocks(job);
}
}  // namespace opentxs::network::blockchain::internal
