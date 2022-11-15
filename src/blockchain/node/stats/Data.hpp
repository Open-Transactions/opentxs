// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>
#include <optional>
#include <string_view>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::stats
{
class Data
{
public:
    using PositionMap = Map<Type, block::Position>;

    PositionMap header_tips_;
    PositionMap block_tips_;
    PositionMap cfilter_tips_;
    PositionMap sync_tips_;
    Map<Type, std::size_t> peer_count_;

    auto Trigger() const noexcept -> void;

    auto Init(const api::Session& api, std::string_view endpoint) noexcept
        -> void;

    Data() noexcept;
    Data(const Data&) = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;

    ~Data();

private:
    mutable std::optional<network::zeromq::socket::Raw> to_actor_;
};
}  // namespace opentxs::blockchain::node::stats
