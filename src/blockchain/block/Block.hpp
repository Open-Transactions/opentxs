// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/block/Block.hpp"

#include <memory>

#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class Header;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::implementation
{
class Block : virtual public internal::Block
{
public:
    auto asBitcoin() const noexcept -> const bitcoin::block::Block& override;
    auto Header() const noexcept -> const block::Header& override;
    auto ID() const noexcept -> const block::Hash& final;

    auto asBitcoin() noexcept -> bitcoin::block::Block& override;

    Block() = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    ~Block() override;

protected:
    Block(const block::Header& header) noexcept;

private:
    const block::Header& base_header_;
    const std::shared_ptr<bitcoin::block::Block> blank_bitcoin_;
};
}  // namespace opentxs::blockchain::block::implementation
