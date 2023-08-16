// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block::script
{
enum class Pattern : std::uint8_t {
    Custom = 0,
    Coinbase,
    NullData,
    PayToMultisig,
    PayToPubkey,
    PayToPubkeyHash,
    PayToScriptHash,
    PayToWitnessPubkeyHash,
    PayToWitnessScriptHash,
    PayToTaproot,
    None = 252,
    Input = 253,
    Empty = 254,
    Malformed = 255,
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::script