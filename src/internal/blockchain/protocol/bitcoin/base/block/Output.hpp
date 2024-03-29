// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
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

class Crypto;
class Session;
}  // namespace api

namespace blockchain
{
namespace protocol
{
namespace bitcoin
{
namespace bitcoincash
{
namespace token
{
namespace cashtoken
{
struct View;
}  // namespace cashtoken
}  // namespace token
}  // namespace bitcoincash
}  // namespace bitcoin
}  // namespace protocol

namespace block
{
class Position;
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace protobuf
{
class BlockchainTransactionOutput;
}  // namespace protobuf

class Amount;
class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
{
class Output
{
public:
    using SerializeType = protobuf::BlockchainTransactionOutput;

    virtual auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        Set<identifier::Nym>& output) const noexcept -> void;
    virtual auto AssociatedRemoteContacts(
        const api::session::Client& api,
        Set<identifier::Generic>& output) const noexcept -> void;
    virtual auto CalculateSize() const noexcept -> std::size_t;
    virtual auto Cashtoken() const noexcept
        -> const bitcoincash::token::cashtoken::View*;
    virtual auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void;
    virtual auto FindMatches(
        const api::Session& api,
        const TransactionHash& txid,
        const cfilter::Type type,
        const ParsedPatterns& elements,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void;
    virtual auto HasKeys() const noexcept -> bool;
    virtual auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Keys(Set<crypto::Key>& out) const noexcept -> void;
    virtual auto Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>;
    virtual auto MinedPosition() const noexcept -> const block::Position&;
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const Log& log) const noexcept -> opentxs::Amount;
    virtual auto Note(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString;
    virtual auto Note(
        const api::crypto::Blockchain& crypto,
        alloc::Default alloc) const noexcept -> CString;
    virtual auto Payee() const noexcept -> ContactID;
    virtual auto Payer() const noexcept -> ContactID;
    virtual auto Print(const api::Crypto& api) const noexcept
        -> UnallocatedCString;
    virtual auto Print(const api::Crypto& api, alloc::Default alloc)
        const noexcept -> CString;
    virtual auto Script() const noexcept -> const block::Script&;
    virtual auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t>;
    virtual auto Serialize(const api::Session& api, SerializeType& destination)
        const noexcept -> bool;
    virtual auto SigningSubscript(alloc::Default alloc) const noexcept
        -> block::Script;
    virtual auto State() const noexcept -> node::TxoState;
    virtual auto Tags() const noexcept -> const UnallocatedSet<node::TxoTag>;
    virtual auto Value() const noexcept -> Amount;

    virtual auto AddTag(node::TxoTag tag) noexcept -> void;
    virtual auto ConfirmMatches(
        const Log& log,
        const api::crypto::Blockchain& api,
        const Matches& candiates) noexcept -> bool;
    virtual auto ForTestingOnlyAddKey(const crypto::Key& key) noexcept -> void;
    virtual auto MergeMetadata(
        const api::Crypto& crypto,
        const Output& rhs,
        const Log& log) noexcept -> void;
    virtual auto RefreshContacts(const api::crypto::Blockchain& api) noexcept
        -> void;
    virtual auto SetIndex(const std::uint32_t index) noexcept -> void;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void;
    virtual auto SetMinedPosition(const block::Position& pos) noexcept -> void;
    virtual auto SetPayee(const identifier::Generic& contact) noexcept -> void;
    virtual auto SetPayer(const identifier::Generic& contact) noexcept -> void;
    virtual auto SetState(node::TxoState state) noexcept -> void;
    virtual auto SetValue(const Amount& value) noexcept -> void;

    virtual ~Output() = default;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
