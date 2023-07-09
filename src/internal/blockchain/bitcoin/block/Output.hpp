// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

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

class Crypto;
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace token
{
namespace cashtoken
{
struct View;
}  // namespace cashtoken
}  // namespace token
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto

class Amount;
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Output
{
public:
    using SerializeType = proto::BlockchainTransactionOutput;
    using ContactID = identifier::Generic;

    virtual auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        Set<identifier::Nym>& output) const noexcept -> void;
    virtual auto AssociatedRemoteContacts(
        const api::session::Client& api,
        Set<identifier::Generic>& output) const noexcept -> void;
    virtual auto CalculateSize() const noexcept -> std::size_t;
    virtual auto Cashtoken() const noexcept -> const token::cashtoken::View*;
    virtual auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void;
    virtual auto FindMatches(
        const api::Session& api,
        const TransactionHash& txid,
        const cfilter::Type type,
        const ParsedPatterns& elements,
        const Log& log,
        Matches& out,
        alloc::Strategy monotonic) const noexcept -> void;
    virtual auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Keys(Set<crypto::Key>& out) const noexcept -> void;
    virtual auto Keys(alloc::Strategy alloc) const noexcept -> Set<crypto::Key>;
    virtual auto MinedPosition() const noexcept -> const block::Position&;
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const Log& log) const noexcept -> opentxs::Amount;
    virtual auto Note(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString;
    virtual auto Note(
        const api::crypto::Blockchain& crypto,
        alloc::Strategy alloc) const noexcept -> CString;
    virtual auto Payee() const noexcept -> ContactID;
    virtual auto Payer() const noexcept -> ContactID;
    virtual auto Print(const api::Crypto& api) const noexcept
        -> UnallocatedCString;
    virtual auto Print(const api::Crypto& api, alloc::Strategy alloc)
        const noexcept -> CString;
    virtual auto Script() const noexcept -> const block::Script&;
    virtual auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t>;
    virtual auto Serialize(const api::Session& api, SerializeType& destination)
        const noexcept -> bool;
    virtual auto SigningSubscript(alloc::Strategy alloc) const noexcept
        -> block::Script;
    virtual auto State() const noexcept -> node::TxoState;
    virtual auto Tags() const noexcept -> const UnallocatedSet<node::TxoTag>;
    virtual auto Value() const noexcept -> Amount;

    virtual auto AddTag(node::TxoTag tag) noexcept -> void;
    virtual auto ForTestingOnlyAddKey(const crypto::Key& key) noexcept -> void;
    virtual auto MergeMetadata(
        const api::Crypto& crypto,
        const Output& rhs,
        const Log& log) noexcept -> void;
    virtual auto SetIndex(const std::uint32_t index) noexcept -> void;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void;
    virtual auto SetMinedPosition(const block::Position& pos) noexcept -> void;
    virtual auto SetPayee(const identifier::Generic& contact) noexcept -> void;
    virtual auto SetPayer(const identifier::Generic& contact) noexcept -> void;
    virtual auto SetState(node::TxoState state) noexcept -> void;
    virtual auto SetValue(const Amount& value) noexcept -> void;

    virtual ~Output() = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
