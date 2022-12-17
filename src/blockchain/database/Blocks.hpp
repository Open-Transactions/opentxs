// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace storage
{
namespace lmdb
{
class Database;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Blocks
{
public:
    auto SetTip(const block::Position& position) const noexcept -> bool;
    auto Tip() const noexcept -> block::Position;

    Blocks(
        const api::Session& api,
        const storage::lmdb::Database& lmdb,
        const blockchain::Type type) noexcept;

private:
    const api::Session& api_;
    const storage::lmdb::Database& lmdb_;
    const block::Position blank_position_;
    const block::Hash genesis_;
};
}  // namespace opentxs::blockchain::database
