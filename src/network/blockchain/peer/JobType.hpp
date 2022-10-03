// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>
#include <variant>

#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/network/blockchain/Peer.hpp"
#include "network/blockchain/peer/Imp.hpp"

namespace opentxs::network::blockchain::internal
{
class Peer::Imp::JobType
{
public:
    static auto get() noexcept -> const JobType&;

    auto operator()(const std::monostate&) const noexcept -> std::string_view;
    auto operator()(const opentxs::blockchain::node::internal::HeaderJob&)
        const noexcept -> std::string_view;
    auto operator()(const opentxs::blockchain::node::internal::BlockBatch&)
        const noexcept -> std::string_view;
    auto operator()(const opentxs::blockchain::node::CfheaderJob&)
        const noexcept -> std::string_view;
    auto operator()(const opentxs::blockchain::node::CfilterJob&) const noexcept
        -> std::string_view;
};
}  // namespace opentxs::network::blockchain::internal
