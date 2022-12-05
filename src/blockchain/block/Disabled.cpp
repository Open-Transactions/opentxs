// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Factory.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::block::blank
{
class Block final : public internal::Block
{
public:
    auto asBitcoin() const noexcept -> const bitcoin::block::Block& final
    {
        static const auto blank = factory::BitcoinBlock();

        return *blank;
    }
    auto CalculateSize() const noexcept -> std::size_t final { return {}; }
    auto ExtractElements(const cfilter::Type, alloc::Default) const noexcept
        -> Elements final
    {
        return {};
    }
    auto FindMatches(
        const api::Session&,
        const cfilter::Type,
        const Patterns&,
        const Patterns&,
        const Log&,
        alloc::Default,
        alloc::Default) const noexcept -> Matches final
    {
        return {};
    }
    auto Header() const noexcept -> const block::Header& final
    {
        static const auto blank = block::Header{};

        return blank;
    }
    auto ID() const noexcept -> const block::Hash& final
    {
        static const auto blank = block::Hash{};

        return blank;
    }
    auto Print() const noexcept -> UnallocatedCString final { return {}; }
    auto Serialize(Writer&&) const noexcept -> bool final { return {}; }

    auto asBitcoin() noexcept -> bitcoin::block::Block& final
    {
        static auto blank = factory::BitcoinBlock();

        return *blank;
    }

    ~Block() final = default;
};
}  // namespace opentxs::blockchain::block::blank

namespace opentxs::factory
{
auto BlockchainBlock(
    const api::Crypto&,
    const blockchain::Type,
    const ReadView) noexcept -> std::shared_ptr<blockchain::block::Block>
{
    return std::make_shared<blockchain::block::blank::Block>();
}
}  // namespace opentxs::factory
