// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"

#pragma once

#include "blockchain/bitcoin/block/parser/Base.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class Parser final : public bitcoin::block::ParserBase
{
public:
    Parser() = delete;
    Parser(
        const api::Crypto& crypto,
        blockchain::Type type,
        alloc::Default alloc) noexcept;
    Parser(const Parser&) noexcept;
    Parser(Parser&&) noexcept;
    auto operator=(const Parser&) -> Parser& = delete;
    auto operator=(Parser&&) -> Parser& = delete;

    ~Parser() final = default;

private:
    auto construct_block(blockchain::block::Block& out) noexcept -> bool final;
};
}  // namespace opentxs::blockchain::bitcoin::block
