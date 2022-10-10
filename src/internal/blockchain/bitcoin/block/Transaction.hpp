// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>
#include <optional>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
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

class Factory;
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
class Output;
}  // namespace internal
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

namespace proto
{
class BlockchainTransaction;
}  // namespace proto

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Transaction : virtual public block::Transaction
{
public:
    using SerializeType = proto::BlockchainTransaction;
    using SigHash = blockchain::bitcoin::SigOption;

    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ConfirmationHeight() const noexcept -> block::Height = 0;
    virtual auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void = 0;
    virtual auto FindMatches(
        const api::Session& api,
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements,
        const Log& log,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> Matches = 0;
    virtual auto FindMatches(
        const api::Session& api,
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void = 0;
    virtual auto GetPreimageBTC(
        const std::size_t index,
        const blockchain::bitcoin::SigHash& hashType) const noexcept
        -> Space = 0;
    virtual auto IndexElements(const api::Session& api, alloc::Default alloc)
        const noexcept -> ElementHashes = 0;
    virtual auto IDNormalized(const api::Factory& factory) const noexcept
        -> const identifier::Generic& = 0;
    auto Internal() const noexcept -> const internal::Transaction& final
    {
        return *this;
    }
    // WARNING do not call this function if another thread has a non-const
    // reference to this object
    virtual auto MinedPosition() const noexcept -> const block::Position& = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(const api::Session& api) const noexcept
        -> std::optional<SerializeType> = 0;

    virtual auto AssociatePreviousOutput(
        const std::size_t inputIndex,
        const Output& output) noexcept -> bool = 0;
    virtual auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool = 0;
    auto Internal() noexcept -> internal::Transaction& final { return *this; }
    virtual auto MergeMetadata(
        const api::crypto::Blockchain& crypto,
        const blockchain::Type chain,
        const Transaction& rhs,
        const Log& log) noexcept -> void = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;
    virtual auto SetMemo(const UnallocatedCString& memo) noexcept -> void = 0;
    virtual auto SetMinedPosition(const block::Position& pos) noexcept
        -> void = 0;
    virtual auto SetPosition(std::size_t position) noexcept -> void = 0;

    ~Transaction() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
