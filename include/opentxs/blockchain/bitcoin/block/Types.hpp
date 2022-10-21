// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::bitcoin::block
{
enum class OP : std::uint8_t;  // IWYU pragma: export

struct ScriptElement {
    using ScriptData = Space;
    using ScriptPushBytes = Space;
    using InvalidOpcode = std::byte;

    OP opcode_{};
    std::optional<InvalidOpcode> invalid_{};
    std::optional<ScriptPushBytes> bytes_{};
    std::optional<ScriptData> data_{};
};

using ScriptElements = UnallocatedVector<ScriptElement>;
}  // namespace opentxs::blockchain::bitcoin::block
