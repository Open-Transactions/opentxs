// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Transaction.hpp"
// IWYU pragma: no_include "opentxs/core/ByteArray.hpp"
// IWYU pragma: no_include "opentxs/core/Data.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Generic.hpp"
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <ankerl/unordered_dense.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <tuple>
#include <utility>

#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
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
class Transaction;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
using AccountID = identifier::Account;
using ContactID = identifier::Generic;
using ElementID = identifier::Generic;
using SubaccountID = identifier::Account;
using SubchainID = identifier::Account;
using SubchainIndex = std::pair<crypto::Subchain, SubaccountID>;
using Element = Vector<std::byte>;
using Elements = Vector<Element>;
using ElementIndex = std::pair<Bip32Index, SubchainIndex>;
using ElementHash = std::uint64_t;
using ElementHashes = Set<ElementHash>;
using Pattern = std::pair<ElementIndex, Element>;
using Patterns = Vector<Pattern>;
using Match = std::pair<TransactionHash, ElementIndex>;
using InputMatch = std::tuple<TransactionHash, Outpoint, ElementIndex>;
using InputMatches = Vector<InputMatch>;
using OutputMatches = Vector<Match>;
using Matches = std::pair<InputMatches, OutputMatches>;
using KeyData = Map<crypto::Key, std::pair<ContactID, ContactID>>;
using TxidIndex =
    ankerl::unordered_dense::pmr::map<TransactionHash, std::size_t>;
using TransactionMap = Vector<Transaction>;

struct ParsedPatterns final : Allocated {
    Elements data_;
    Map<ReadView, Patterns::const_iterator> map_;

    auto get_allocator() const noexcept -> allocator_type final;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

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
    const TransactionHash& txid,
    const ParsedPatterns& patterns,
    const Elements& compare,
    alloc::Default alloc,
    alloc::Default monotonic) noexcept -> Matches;
auto SetIntersection(
    const TransactionHash& txid,
    const ParsedPatterns& patterns,
    const Elements& compare,
    std::function<void(const Match&)> cb,
    Matches& out,
    alloc::Default monotonic) noexcept -> void;
}  // namespace opentxs::blockchain::block::internal
