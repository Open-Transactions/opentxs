// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "internal/network/blockchain/Peer.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/LogMacros.hpp"
#include "network/blockchain/peer/Imp.hpp"

namespace opentxs::network::blockchain::internal
{
Peer::Peer(boost::shared_ptr<Imp>&& imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(imp_);
}

Peer::Peer(Peer&& rhs) noexcept
    : Peer(std::move(rhs.imp_))
{
}

auto Peer::Start() noexcept -> void { imp_->Init(imp_); }

Peer::~Peer() = default;
}  // namespace opentxs::network::blockchain::internal
