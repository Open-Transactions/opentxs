// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <limits>

#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
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
class Header;
class Position;
}  // namespace block

namespace database
{
class Header;
}  // namespace database

namespace node
{
class Manager;
class UpdateTransaction;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
struct HeaderOraclePrivate {
    const api::Session& api_;
    const blockchain::Type chain_;
    const CString endpoint_;
    const block::Height checkpoint_height_;
    database::Header& database_;
    network::zeromq::socket::Raw to_parent_;
    network::zeromq::socket::Raw to_actor_;
    block::Position best_;
    bool have_outstanding_job_;

    static auto Genesis(blockchain::Type chain) noexcept
        -> const block::Position&;

    auto Genesis() const noexcept -> const block::Position&;
    auto IsSynchronized() const noexcept -> bool;
    auto JobIsAvailable() const noexcept -> bool;
    auto Remote() const noexcept { return effective_remote_height_; }
    auto Target() const noexcept -> block::Height;

    auto AddUnknownHash(const block::Hash& hash) noexcept -> void;
    auto PruneKnownHashes() noexcept -> void;
    auto UpdateRemoteHeight(block::Height value) noexcept -> bool;

    HeaderOraclePrivate(
        const api::Session& api,
        const node::Manager& node) noexcept;
    HeaderOraclePrivate() = delete;
    HeaderOraclePrivate(const HeaderOraclePrivate&) = delete;
    HeaderOraclePrivate(HeaderOraclePrivate&&) = delete;
    auto operator=(const HeaderOraclePrivate&) -> HeaderOraclePrivate& = delete;
    auto operator=(HeaderOraclePrivate&&) -> HeaderOraclePrivate& = delete;

    ~HeaderOraclePrivate();

private:
    static constexpr auto max_height_ =
        std::numeric_limits<block::Height>::max();

    Map<block::Height, Set<block::Hash>> unknown_hashes_;
    Map<block::Hash, block::Height> unknown_hash_index_;
    block::Height highest_peer_height_;
    block::Height effective_remote_height_;

    auto prune_unknown() noexcept -> void;
};
}  // namespace opentxs::blockchain::node::internal
