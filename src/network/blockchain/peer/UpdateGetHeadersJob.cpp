// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "network/blockchain/peer/UpdateGetHeadersJob.hpp"  // IWYU pragma: associated

namespace opentxs::network::blockchain::internal
{
auto Peer::Imp::UpdateGetHeadersJob::operator()(
    std::monostate& job) const noexcept -> JobUpdate
{
    return {false, false};
}

auto Peer::Imp::UpdateGetHeadersJob::operator()(
    opentxs::blockchain::node::internal::HeaderJob& job) const noexcept
    -> JobUpdate
{
    return {true, true};
}

auto Peer::Imp::UpdateGetHeadersJob::operator()(
    opentxs::blockchain::node::internal::BlockBatch& job) const noexcept
    -> JobUpdate
{
    return {false, false};
}
}  // namespace opentxs::network::blockchain::internal
