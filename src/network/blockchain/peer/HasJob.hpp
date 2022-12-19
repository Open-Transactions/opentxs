// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <variant>

#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/network/blockchain/Peer.hpp"
#include "network/blockchain/peer/Imp.hpp"

namespace opentxs::network::blockchain::internal
{
class Peer::Imp::HasJob
{
public:
    auto operator()(const std::monostate&) const noexcept -> bool;
    auto operator()(const opentxs::blockchain::node::internal::HeaderJob&)
        const noexcept -> bool;
    auto operator()(const opentxs::blockchain::node::internal::BlockBatch&)
        const noexcept -> bool;
};
}  // namespace opentxs::network::blockchain::internal
