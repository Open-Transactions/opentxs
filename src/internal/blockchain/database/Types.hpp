// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <tuple>

#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
class Hash;
class Header;
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
// parent hash, child hash
using ChainSegment = std::pair<block::Hash, block::Hash>;
using UpdatedHeader = UnallocatedMap<
    block::Hash,
    std::pair<std::unique_ptr<block::Header>, bool>>;
using BestHashes = UnallocatedMap<block::Height, block::Hash>;
using Hashes = UnallocatedSet<block::Hash>;
using HashVector = Vector<block::Hash>;
using Segments = UnallocatedSet<ChainSegment>;
// parent block hash, disconnected block hash
using DisconnectedList = UnallocatedMultimap<block::Hash, block::Hash>;

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
}  // namespace opentxs::blockchain::database
