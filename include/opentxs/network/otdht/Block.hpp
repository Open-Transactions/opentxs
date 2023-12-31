// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace protobuf
{
class P2PBlockchainSync;
}  // namespace protobuf

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class OPENTXS_EXPORT Block
{
public:
    auto Chain() const noexcept -> opentxs::blockchain::Type;
    auto Filter() const noexcept -> ReadView;
    auto FilterElements() const noexcept -> std::uint32_t;
    auto FilterType() const noexcept -> opentxs::blockchain::cfilter::Type;
    auto Header() const noexcept -> ReadView;
    auto Height() const noexcept -> opentxs::blockchain::block::Height;
    OPENTXS_NO_EXPORT auto Serialize(
        protobuf::P2PBlockchainSync& dest) const noexcept -> bool;
    OPENTXS_NO_EXPORT auto Serialize(Writer&& dest) const noexcept -> bool;

    OPENTXS_NO_EXPORT Block(
        const protobuf::P2PBlockchainSync& serialized) noexcept(false);
    OPENTXS_NO_EXPORT Block(
        opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Height height,
        opentxs::blockchain::cfilter::Type type,
        std::uint32_t count,
        ReadView header,
        ReadView filter) noexcept(false);
    Block() noexcept = delete;
    Block(const Block& rhs) noexcept;
    OPENTXS_NO_EXPORT Block(Block&&) noexcept;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    OPENTXS_NO_EXPORT ~Block();

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::network::otdht
