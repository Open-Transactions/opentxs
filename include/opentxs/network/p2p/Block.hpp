// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/FilterType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class BlockchainP2PSync;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::p2p
{
class OPENTXS_EXPORT Block
{
public:
    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto Filter() const noexcept -> ReadView;
    auto FilterElements() const noexcept -> std::uint32_t;
    auto FilterType() const noexcept -> opentxs::blockchain::filter::Type;
    auto Header() const noexcept -> ReadView;
    auto Height() const noexcept -> opentxs::blockchain::block::Height;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::BlockchainP2PSync& dest) const noexcept -> bool;
    OPENTXS_NO_EXPORT auto Serialize(AllocateOutput dest) const noexcept
        -> bool;

    OPENTXS_NO_EXPORT Block(
        const proto::BlockchainP2PSync& serialized) noexcept(false);
    OPENTXS_NO_EXPORT Block(
        opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Height height,
        opentxs::blockchain::filter::Type type,
        std::uint32_t count,
        ReadView header,
        ReadView filter) noexcept(false);
    OPENTXS_NO_EXPORT Block(Block&&) noexcept;

    OPENTXS_NO_EXPORT ~Block();

private:
    struct Imp;

    Imp* imp_;

    Block() noexcept;
    Block(const Block&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace opentxs::network::p2p
