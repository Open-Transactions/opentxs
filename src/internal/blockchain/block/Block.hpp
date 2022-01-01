// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Header;
struct Outpoint;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::block
{
using Subchain = blockchain::crypto::Subchain;
using SubchainID = std::pair<Subchain, OTIdentifier>;
using ElementID = std::pair<Bip32Index, SubchainID>;
using Pattern = std::pair<ElementID, Space>;
using Patterns = std::pmr::vector<Pattern>;
using Match = std::pair<pTxid, ElementID>;
using InputMatch = std::tuple<pTxid, Outpoint, ElementID>;
using InputMatches = std::pmr::vector<InputMatch>;
using OutputMatches = std::pmr::vector<Match>;
using Matches = std::pair<InputMatches, OutputMatches>;
using KeyID = blockchain::crypto::Key;
using ContactID = OTIdentifier;
using KeyData = std::pmr::map<KeyID, std::pair<ContactID, ContactID>>;

struct ParsedPatterns {
    std::pmr::vector<Space> data_;
    std::pmr::map<ReadView, Patterns::const_iterator> map_;

    ParsedPatterns(const Patterns& in) noexcept;
};
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::internal
{
struct Block : virtual public block::Block {
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> std::pmr::vector<Space> = 0;
    virtual auto FindMatches(
        const filter::Type type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches = 0;

    ~Block() override = default;
};

auto SetIntersection(
    const api::Session& api,
    const ReadView txid,
    const ParsedPatterns& patterns,
    const std::pmr::vector<Space>& compare) noexcept -> Matches;
}  // namespace opentxs::blockchain::block::internal

namespace opentxs::factory
{
auto GenesisBlockHeader(
    const api::Session& api,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::block::Header>;
}  // namespace opentxs::factory
