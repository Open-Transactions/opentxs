// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/database/Blocks.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <memory>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/LMDB.hpp"

namespace opentxs::blockchain::database
{
Blocks::Blocks(
    const api::Session& api,
    const common::Database& common,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_(make_blank<block::Position>::value(api))
    , chain_(type)
    , genesis_([&] {
        const auto& hex = params::Data::Chains().at(chain_).genesis_hash_hex_;

        return api_.Factory().Data(hex, StringStyle::Hex);
    }())
{
    if (blank_position_.first == Tip().first) {
        SetTip(block::Position{0, genesis_});
    }
}

auto Blocks::LoadBitcoin(const block::Hash& block) const noexcept
    -> std::shared_ptr<const block::bitcoin::Block>
{
    if (block == genesis_) {
        const auto& hex = params::Data::Chains().at(chain_).genesis_block_hex_;
        const auto data = api_.Factory().Data(hex, StringStyle::Hex);

        if (data->empty()) {
            LogError()(OT_PRETTY_CLASS())("Invalid genesis hex").Flush();

            return {};
        }

        return api_.Factory().BitcoinBlock(chain_, data->Bytes());
    } else {
        const auto bytes = common_.BlockLoad(block);

        if (false == bytes.valid()) {
            LogDebug()(OT_PRETTY_CLASS())("block ")(block.asHex())(
                " not found.")
                .Flush();

            return {};
        }

        return api_.Factory().BitcoinBlock(chain_, bytes.get());
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
    const auto size = block.Internal().CalculateSize();
    auto writer = common_.BlockStore(block.ID(), size);

    if (false == writer.get().valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate storage for block")
            .Flush();

        return false;
    }

    if (false ==
        block.Serialize(preallocated(writer.size(), writer.get().data()))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize block").Flush();

        return false;
    }

    return true;
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
