// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <utility>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::block::bitcoin
{
enum class OP : std::uint8_t;

struct ScriptElement {
    using ScriptData = Space;
    using ScriptPushBytes = Space;
    using InvalidOpcode = std::byte;

    OP opcode_{};
    std::optional<InvalidOpcode> invalid_{};
    std::optional<ScriptPushBytes> bytes_{};
    std::optional<ScriptData> data_{};

    auto operator==(const ScriptElement& other) const noexcept -> bool
    {
        return opcode_ == other.opcode_ && invalid_ == other.invalid_ &&
               bytes_ == other.bytes_ && data_ == other.data_;
    }
    auto operator!=(const ScriptElement& other) const noexcept -> bool
    {
        return !(*this == other);
    }
};

using ScriptElements = UnallocatedVector<ScriptElement>;
}  // namespace opentxs::blockchain::block::bitcoin
