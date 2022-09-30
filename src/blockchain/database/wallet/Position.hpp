// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <mutex>
#include <optional>

#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Bytes.hpp"

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

namespace opentxs::blockchain::database::wallet::db
{
struct Position {
    const Space data_;

    auto Decode(const api::Session& api) const noexcept
        -> const block::Position&;
    auto Hash() const noexcept -> ReadView;
    auto Height() const noexcept -> block::Height;

    Position(const block::Position& position) noexcept;
    Position(const ReadView bytes) noexcept(false);
    Position() = delete;
    Position(const Position&) = delete;
    Position(Position&&) = delete;
    auto operator=(const Position&) -> Position& = delete;
    auto operator=(Position&&) -> Position& = delete;

    ~Position() = default;

private:
    static constexpr auto fixed_ = sizeof(block::Height) + 32_uz;

    mutable std::mutex lock_;
    mutable std::optional<block::Position> position_;
};
}  // namespace opentxs::blockchain::database::wallet::db
