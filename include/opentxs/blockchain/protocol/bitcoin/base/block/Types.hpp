// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
class TransactionHash;
class Outpoint;
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
using blockchain::block::Hash;
using blockchain::block::Height;
using blockchain::block::Outpoint;
using blockchain::block::Position;
using blockchain::block::TransactionHash;
using WitnessItem = ByteArray;
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block

namespace opentxs::blockchain::protocol::bitcoin::base::block::script
{
enum class OP : std::uint8_t;        // IWYU pragma: export
enum class Pattern : std::uint8_t;   // IWYU pragma: export
enum class Position : std::uint8_t;  // IWYU pragma: export

constexpr auto value(OP i) noexcept { return static_cast<std::uint8_t>(i); }
OPENTXS_EXPORT auto print(blockchain::Type chain, OP) noexcept
    -> std::string_view;

struct OPENTXS_EXPORT Element {
    using Data = ByteArray;
    using PushArg = ByteArray;
    using InvalidOpcode = std::uint8_t;

    OP opcode_{};
    std::optional<InvalidOpcode> invalid_{};
    std::optional<PushArg> push_bytes_{};
    std::optional<Data> data_{};
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::script
