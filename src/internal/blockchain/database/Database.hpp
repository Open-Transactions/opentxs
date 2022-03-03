// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "api/client/blockchain/database/Database.hpp"

#pragma once

#include <cstdint>
#include <shared_mutex>

#include "opentxs/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs::blockchain::database
{
using BlockReader = ProtectedView<ReadView, std::shared_mutex, sLock>;
using BlockWriter = ProtectedView<WritableView, std::shared_mutex, eLock>;

enum Table {
    Config = 0,
    BlockHeaderMetadata = 1,
    BlockHeaderBest = 2,
    ChainData = 3,
    BlockHeaderSiblings = 4,
    BlockHeaderDisconnected = 5,
    BlockFilterBest = 6,
    BlockFilterHeaderBest = 7,
    Proposals = 8,
    SubchainLastIndexed = 9,
    SubchainLastScanned = 10,
    SubchainID = 11,
    WalletPatterns = 12,
    SubchainPatterns = 13,
    SubchainMatches = 14,
    WalletOutputs = 15,
    AccountOutputs = 16,
    NymOutputs = 17,
    PositionOutputs = 18,
    ProposalCreatedOutputs = 19,
    ProposalSpentOutputs = 20,
    OutputProposals = 21,
    StateOutputs = 22,
    SubchainOutputs = 23,
    KeyOutputs = 24,
    GenerationOutputs = 25,
};

enum class Key : std::size_t {
    Version = 0,
    TipHeight = 1,
    CheckpointHeight = 2,
    CheckpointHash = 3,
    BestFullBlock = 4,
    SyncPosition = 5,
    WalletPosition = 6,
};

enum class BlockStorage : std::uint8_t {
    None = 0,
    Cache = 1,
    All = 2,
};
}  // namespace opentxs::blockchain::database
