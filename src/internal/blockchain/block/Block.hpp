// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <tuple>
#include <utility>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Header;
class Outpoint;
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
using Subchain = blockchain::crypto::Subchain;
using SubchainID = std::pair<Subchain, OTIdentifier>;
using ElementID = std::pair<Bip32Index, SubchainID>;
using Pattern = std::pair<ElementID, Space>;
using Patterns = UnallocatedVector<Pattern>;
using Match = std::pair<pTxid, ElementID>;
using InputMatch = std::tuple<pTxid, Outpoint, ElementID>;
using InputMatches = UnallocatedVector<InputMatch>;
using OutputMatches = UnallocatedVector<Match>;
using Matches = std::pair<InputMatches, OutputMatches>;
using KeyID = blockchain::crypto::Key;
using ContactID = OTIdentifier;
using KeyData = UnallocatedMap<KeyID, std::pair<ContactID, ContactID>>;

struct ParsedPatterns {
    UnallocatedVector<Space> data_;
    UnallocatedMap<ReadView, Patterns::const_iterator> map_;

    ParsedPatterns(const Patterns& in) noexcept;
};
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::internal
{
struct Block : virtual public block::Block {
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ExtractElements(const cfilter::Type style) const noexcept
        -> UnallocatedVector<Space> = 0;
    virtual auto FindMatches(
        const cfilter::Type type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches = 0;

    ~Block() override = default;
};

auto SetIntersection(
    const api::Session& api,
    const ReadView txid,
    const ParsedPatterns& patterns,
    const UnallocatedVector<Space>& compare) noexcept -> Matches;
}  // namespace opentxs::blockchain::block::internal

namespace opentxs::factory
{
auto GenesisBlockHeader(
    const api::Session& api,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::block::Header>;
}  // namespace opentxs::factory
