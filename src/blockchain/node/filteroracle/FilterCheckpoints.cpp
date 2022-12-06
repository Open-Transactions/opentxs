// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/filteroracle/FilterOracle.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::node::implementation
{
const FilterOracle::CheckpointMap FilterOracle::filter_checkpoints_{};
}  // namespace opentxs::blockchain::node::implementation
