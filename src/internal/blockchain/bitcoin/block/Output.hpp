// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoState
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoTag
// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Script;
}  // namespace internal
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto

class Amount;
class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Output : virtual public block::Output
{
public:
    using SerializeType = proto::BlockchainTransactionOutput;

    virtual auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        UnallocatedVector<identifier::Nym>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        const api::session::Client& api,
        UnallocatedVector<identifier::Generic>& output) const noexcept
        -> void = 0;
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Output> = 0;
    virtual auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void = 0;
    virtual auto FindMatches(
        const api::Session& api,
        const Txid& txid,
        const cfilter::Type type,
        const ParsedPatterns& elements,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void = 0;
    virtual auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void = 0;
    auto Internal() const noexcept -> const internal::Output& final
    {
        return *this;
    }
    // WARNING do not call this function if another thread has a non-const
    // reference to this object
    virtual auto MinedPosition() const noexcept -> const block::Position& = 0;
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const Log& log) const noexcept -> opentxs::Amount = 0;
    virtual auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(const api::Session& api, SerializeType& destination)
        const noexcept -> bool = 0;
    virtual auto SigningSubscript() const noexcept
        -> std::unique_ptr<internal::Script> = 0;
    virtual auto State() const noexcept -> node::TxoState = 0;
    virtual auto Tags() const noexcept
        -> const UnallocatedSet<node::TxoTag> = 0;

    virtual auto AddTag(node::TxoTag tag) noexcept -> void = 0;
    virtual auto ForTestingOnlyAddKey(const crypto::Key& key) noexcept
        -> void = 0;
    auto Internal() noexcept -> internal::Output& final { return *this; }
    virtual auto MergeMetadata(const Output& rhs, const Log& log) noexcept
        -> bool = 0;
    virtual auto SetIndex(const std::uint32_t index) noexcept -> void = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;
    virtual auto SetMinedPosition(const block::Position& pos) noexcept
        -> void = 0;
    virtual auto SetPayee(const identifier::Generic& contact) noexcept
        -> void = 0;
    virtual auto SetPayer(const identifier::Generic& contact) noexcept
        -> void = 0;
    virtual auto SetState(node::TxoState state) noexcept -> void = 0;
    virtual auto SetValue(const blockchain::Amount& value) noexcept -> void = 0;

    ~Output() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
