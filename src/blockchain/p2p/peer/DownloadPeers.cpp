// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::p2p::implementation
{
Peer::DownloadPeers::DownloadPeers() noexcept
    : downloaded_(Clock::now())
{
}

auto Peer::DownloadPeers::Bump() noexcept -> void
{
    downloaded_ = Clock::now();
}

auto Peer::DownloadPeers::get() const noexcept -> Time { return downloaded_; }
}  // namespace opentxs::blockchain::p2p::implementation
