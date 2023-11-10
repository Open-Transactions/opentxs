// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/Log.hpp"
// IWYU pragma: no_include <boost/container/container_fwd.hpp>
// IWYU pragma: no_include <boost/intrusive/detail/std_fwd.hpp>

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/unordered/unordered_node_map.hpp>
#include <boost/unordered/unordered_node_set.hpp>
#include <functional>
#include <utility>

#include "internal/blockchain/database/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Outpoint;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Output;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet
{
enum class ProposalAssociation { none, created, consumed };

constexpr auto accounts_{Table::AccountOutputs};
constexpr auto generation_{Table::GenerationOutputs};
constexpr auto keys_{Table::KeyOutputs};
constexpr auto nyms_{Table::NymOutputs};
constexpr auto output_config_{Table::Config};
constexpr auto output_proposal_{Table::OutputProposals};
constexpr auto outputs_{Table::WalletOutputs};
constexpr auto positions_{Table::PositionOutputs};
constexpr auto proposal_created_{Table::ProposalCreatedOutputs};
constexpr auto proposal_spent_{Table::ProposalSpentOutputs};
constexpr auto states_{Table::StateOutputs};
constexpr auto subchains_{Table::SubchainOutputs};

template <typename Key, typename Value>
using FlatMap = boost::container::
    flat_map<Key, Value, std::less<Key>, alloc::PMR<std::pair<Key, Value>>>;
template <typename Value>
using FlatSet =
    boost::container::flat_set<Value, std::less<Value>, alloc::PMR<Value>>;
template <typename Key, typename Value>
using MapType = boost::unordered_node_map<Key, Value, std::hash<Key>>;
template <typename Value>
using SetType = boost::unordered_node_set<Value, std::hash<Value>>;
using States = UnallocatedVector<node::TxoState>;
using Matches = UnallocatedVector<block::Outpoint>;
using Outpoints = Set<block::Outpoint>;
using Nyms = SetType<identifier::Nym>;
using OrphanedGeneration = Set<block::Outpoint>;
using Reserved = Outpoints;
using ParsedTXOs =
    Map<block::Outpoint,
        std::pair<
            protocol::bitcoin::base::block::Output,
            Vector<identifier::Generic>>>;

auto all_states() noexcept -> const States&;
auto is_mature(
    const block::Height minedAt,
    const block::Height bestChain,
    const block::Height target) noexcept -> bool;
}  // namespace opentxs::blockchain::database::wallet
