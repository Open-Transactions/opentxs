// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "blockchain/database/Blocks.hpp"  // IWYU pragma: associated

#include <cstddef>

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::blockchain::database
{
Blocks::Blocks(
    const api::Session& api,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , blank_position_()
    , genesis_(params::get(type).GenesisHash())
{
    if (blank_position_.height_ == Tip().height_) {
        SetTip(block::Position{0, genesis_});
    }
}

auto Blocks::SetTip(const block::Position& position) const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::Config,
            tsv(static_cast<std::size_t>(Key::BestFullBlock)),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Blocks::Tip() const noexcept -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(
        Table::Config, tsv(static_cast<std::size_t>(Key::BestFullBlock)), cb);

    return output;
}
}  // namespace opentxs::blockchain::database
