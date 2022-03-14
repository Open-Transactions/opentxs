// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/util/Iterator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::bitcoin
{
class OPENTXS_EXPORT Block : virtual public block::Block
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
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Block() override = default;

protected:
    Block() noexcept = default;

private:
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin
