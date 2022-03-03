// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/block/Block.hpp"

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Header.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::implementation
{
class Block : virtual public internal::Block
{
public:
    auto Header() const noexcept -> const block::Header& override
    {
        return base_header_;
    }
    auto ID() const noexcept -> const block::Hash& final
    {
        return base_header_.Hash();
    }
    auto Internal() const noexcept -> const internal::Block& final
    {
        return *this;
    }

    auto Internal() noexcept -> internal::Block& final { return *this; }

protected:
    const api::Session& api_;

    Block(const api::Session& api, const block::Header& header) noexcept;

private:
    const block::Header& base_header_;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace opentxs::blockchain::block::implementation
