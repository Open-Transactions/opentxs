// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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
namespace bitcoin
{
namespace block
{
class Output;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Outpoint;
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionInput;
}  // namespace proto

class Amount;
class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Input
{
public:
    using SerializeType = proto::BlockchainTransactionInput;
    using Signature = std::pair<ReadView, ReadView>;
    using Signatures = UnallocatedVector<Signature>;

    virtual auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        Set<identifier::Nym>& output) const noexcept -> void;
    virtual auto AssociatedRemoteContacts(
        const api::session::Client& api,
        Set<identifier::Generic>& output) const noexcept -> void;
    virtual auto CalculateSize(const bool normalized = false) const noexcept
        -> std::size_t;
    virtual auto Coinbase() const noexcept -> ReadView;
    virtual auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void;
    virtual auto FindMatches(
        const api::Session& api,
        const TransactionHash& txid,
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements,
        const std::size_t position,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void;
    virtual auto GetBytes(std::size_t& base, std::size_t& witness)
        const noexcept -> void;
    virtual auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Keys(Set<crypto::Key>& out) const noexcept -> void;
    virtual auto Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key>;
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const std::size_t index,
        const Log& log) const noexcept -> opentxs::Amount;
    virtual auto PreviousOutput() const noexcept
        -> const blockchain::block::Outpoint&;
    virtual auto Print(const api::Crypto& crypto) const noexcept
        -> UnallocatedCString;
    virtual auto Print(const api::Crypto& crypto, alloc::Default alloc)
        const noexcept -> CString;
    virtual auto Script() const noexcept -> const block::Script&;
    virtual auto Sequence() const noexcept -> std::uint32_t;
    virtual auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t>;
    virtual auto Serialize(
        const api::Session& api,
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool;
    virtual auto SerializeNormalized(Writer&& destination) const noexcept
        -> std::optional<std::size_t>;
    virtual auto SignatureVersion(alloc::Default alloc) const noexcept
        -> block::Input;
    virtual auto SignatureVersion(block::Script subscript, alloc::Default alloc)
        const noexcept -> block::Input;
    virtual auto Spends() const noexcept(false) -> const block::Output&;
    virtual auto Witness() const noexcept -> std::span<const WitnessItem>;

    virtual auto AddMultisigSignatures(const Signatures& signatures) noexcept
        -> bool;
    virtual auto AddSignatures(const Signatures& signatures) noexcept -> bool;
    virtual auto AssociatePreviousOutput(const block::Output& output) noexcept
        -> bool;
    virtual auto MergeMetadata(
        const api::Crypto& crypto,
        const Input& rhs,
        const std::size_t index,
        const Log& log) noexcept -> void;
    virtual auto ReplaceScript() noexcept -> bool;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void;

    virtual ~Input() = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
