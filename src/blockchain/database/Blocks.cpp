// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "blockchain/database/Blocks.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <memory>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/block/Block.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::database
{
Blocks::Blocks(
    const api::Session& api,
    const common::Database& common,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_()
    , chain_(type)
    , genesis_(params::get(type).GenesisHash())
{
    if (blank_position_.height_ == Tip().height_) {
        SetTip(block::Position{0, genesis_});
    }
}

auto Blocks::LoadBitcoin(const block::Hash& block) const noexcept
    -> std::shared_ptr<const bitcoin::block::Block>
{
    const auto& log = LogDebug();

    if (block == genesis_) {
        log(OT_PRETTY_CLASS())("loading genesis block").Flush();

        return params::get(chain_)
            .GenesisBlock()
            .asBitcoin()
            .InternalBitcoin()
            .clone_bitcoin();
    } else {
        const auto bytes = common_.BlockLoad(block);

        if (false == valid(bytes)) {
            log(OT_PRETTY_CLASS())("block ")(block.asHex())(" not found.")
                .Flush();

            return {};
        }

        if (auto pBlock = api_.Factory().BitcoinBlock(chain_, bytes); pBlock) {

            return pBlock;
        } else {
            LogError()(OT_PRETTY_CLASS())("error parsing block ")
                .asHex(block)
                .Flush();
            common_.BlockForget(block);

            return {};
        }
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

auto Blocks::Store(const block::Block& block) const noexcept -> bool
{
    return common_.BlockStore(block);
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
