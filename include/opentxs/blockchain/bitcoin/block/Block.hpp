// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/util/Iterator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Block;
}  // namespace internal

class Transaction;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Block : virtual public blockchain::block::Block
{
public:
    using value_type = std::shared_ptr<const Transaction>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Block, const value_type>;

    virtual auto at(const std::size_t index) const noexcept
        -> const value_type& = 0;
    virtual auto at(const ReadView txid) const noexcept
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    OPENTXS_NO_EXPORT virtual auto InternalBitcoin() const noexcept
        -> const internal::Block& = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    OPENTXS_NO_EXPORT virtual auto InternalBitcoin() noexcept
        -> internal::Block& = 0;

    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    ~Block() override = default;

protected:
    Block() noexcept = default;
};
}  // namespace opentxs::blockchain::bitcoin::block
