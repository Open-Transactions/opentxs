// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Data.hpp"

#pragma once

#include <cstdint>
#include <optional>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Types.hpp"
#include "opentxs/core/ByteArray.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::script
{
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
