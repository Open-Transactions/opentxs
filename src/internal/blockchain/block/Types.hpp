// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <tuple>
#include <utility>

#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Outpoint;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
using AccountID = identifier::Generic;
using ContactID = identifier::Generic;
using ElementID = identifier::Generic;
using SubaccountID = identifier::Generic;
using SubchainID = identifier::Generic;
using SubchainIndex = std::pair<crypto::Subchain, SubaccountID>;
using Element = Vector<std::byte>;
using Elements = Vector<Element>;
using ElementIndex = std::pair<Bip32Index, SubchainIndex>;
using ElementHash = std::uint64_t;
using ElementHashes = Set<ElementHash>;
using Pattern = std::pair<ElementIndex, Element>;
using Patterns = Vector<Pattern>;
using Match = std::pair<pTxid, ElementIndex>;
using InputMatch = std::tuple<pTxid, Outpoint, ElementIndex>;
using InputMatches = Vector<InputMatch>;
using OutputMatches = Vector<Match>;
using Matches = std::pair<InputMatches, OutputMatches>;
using KeyData = UnallocatedMap<crypto::Key, std::pair<ContactID, ContactID>>;

struct ParsedPatterns final : Allocated {
    Elements data_;
    Map<ReadView, Patterns::const_iterator> map_;

    auto get_allocator() const noexcept -> allocator_type final;

    ParsedPatterns(const Patterns& in, allocator_type alloc) noexcept;

    ParsedPatterns() = delete;
    ParsedPatterns(const ParsedPatterns&) = delete;
    ParsedPatterns(ParsedPatterns&&) = delete;
    auto operator=(const ParsedPatterns&) -> ParsedPatterns& = delete;
    auto operator=(ParsedPatterns&&) -> ParsedPatterns& = delete;

    ~ParsedPatterns() final = default;
};
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::internal
{
auto SetIntersection(
    const ReadView txid,
    const ParsedPatterns& patterns,
    const Elements& compare,
    alloc::Default alloc,
    alloc::Default monotonic) noexcept -> Matches;
auto SetIntersection(
    const ReadView txid,
    const ParsedPatterns& patterns,
    const Elements& compare,
    std::function<void(const Match&)> cb,
    Matches& out,
    alloc::Default monotonic) noexcept -> void;
}  // namespace opentxs::blockchain::block::internal
