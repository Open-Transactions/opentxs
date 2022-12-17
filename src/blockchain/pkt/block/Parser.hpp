// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"

#pragma once

#include <cstddef>
#include <memory>
#include <utility>

#include "blockchain/bitcoin/block/parser/Base.hpp"
#include "internal/blockchain/pkt/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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

namespace network
{
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::pkt::block
{
using network::blockchain::bitcoin::CompactSize;

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

protected:
    auto construct_block(blockchain::block::Block& out) noexcept -> bool final;
    auto find_payload() noexcept -> bool final;

private:
    Proofs proofs_;
    std::size_t proof_bytes_;
};
}  // namespace opentxs::blockchain::pkt::block
