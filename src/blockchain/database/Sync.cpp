// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/Sync.hpp"  // IWYU pragma: associated

#include <cstddef>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::database::implementation
{
Sync::Sync(
    const api::Session& api,
    const common::Database& common,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_()
    , chain_(type)
    , genesis_(params::get(chain_).GenesisHash())
{
    auto tip = Tip();

    if (blank_position_.height_ == tip.height_) {
        const auto genesis = block::Position{0, genesis_};
        const auto saved = SetTip(genesis);
        tip = genesis;

        OT_ASSERT(saved);
    }

    LogVerbose()(OT_PRETTY_CLASS())("Sync tip: ")(tip.height_).Flush();

    if (const auto ctip = common_.SyncTip(chain_); tip.height_ == ctip) {
        LogVerbose()(OT_PRETTY_CLASS())("Database is consistent").Flush();
    } else {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Database inconsistency detected. Storage tip height: ")(ctip)
            .Flush();
    }
}

auto Sync::Load(const block::Height height, Message& output) const noexcept
    -> bool
{
    return common_.LoadSync(chain_, height, output);
}

auto Sync::Reorg(const block::Height height) const noexcept -> bool
{
    return common_.ReorgSync(chain_, height);
}

auto Sync::SetTip(const block::Position& position) const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::Config,
            tsv(static_cast<std::size_t>(Key::SyncPosition)),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Sync::Store(
    const block::Position& tip,
    const network::otdht::SyncData& items) const noexcept -> bool
{
    if (false == common_.StoreSync(items, chain_)) {
        LogError()(OT_PRETTY_CLASS())("Failed to store sync data").Flush();

        return false;
    }

    return SetTip(tip);
}

auto Sync::Tip() const noexcept -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(
        Table::Config, tsv(static_cast<std::size_t>(Key::SyncPosition)), cb);

    return output;
}
}  // namespace opentxs::blockchain::database::implementation
