// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
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
class BlockchainTransactionInput;
}  // namespace proto

class Amount;
class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::internal
{
class Input : virtual public block::Input
{
public:
    using SerializeType = proto::BlockchainTransactionInput;
    using Signature = std::pair<ReadView, ReadView>;
    using Signatures = UnallocatedVector<Signature>;

    virtual auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        UnallocatedVector<identifier::Nym>& output) const noexcept -> void = 0;
    virtual auto AssociatedRemoteContacts(
        const api::session::Client& api,
        UnallocatedVector<identifier::Generic>& output) const noexcept
        -> void = 0;
    virtual auto CalculateSize(const bool normalized = false) const noexcept
        -> std::size_t = 0;
    virtual auto clone() const noexcept -> std::unique_ptr<Input> = 0;
    virtual auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void = 0;
    virtual auto FindMatches(
        const api::Session& api,
        const Txid& txid,
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements,
        const std::size_t position,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void = 0;
    virtual auto GetBytes(std::size_t& base, std::size_t& witness)
        const noexcept -> void = 0;
    virtual auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void = 0;
    auto Internal() const noexcept -> const internal::Input& final
    {
        return *this;
    }
    virtual auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const std::size_t index,
        const Log& log) const noexcept -> opentxs::Amount = 0;
    virtual auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(
        const api::Session& api,
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool = 0;
    virtual auto SerializeNormalized(Writer&& destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto SignatureVersion() const noexcept
        -> std::unique_ptr<Input> = 0;
    virtual auto SignatureVersion(std::unique_ptr<internal::Script> subscript)
        const noexcept -> std::unique_ptr<Input> = 0;
    virtual auto Spends() const noexcept(false) -> const Output& = 0;

    virtual auto AddMultisigSignatures(const Signatures& signatures) noexcept
        -> bool = 0;
    virtual auto AddSignatures(const Signatures& signatures) noexcept
        -> bool = 0;
    virtual auto AssociatePreviousOutput(const Output& output) noexcept
        -> bool = 0;
    auto Internal() noexcept -> internal::Input& final { return *this; }
    virtual auto MergeMetadata(
        const Input& rhs,
        const std::size_t index,
        const Log& log) noexcept -> bool = 0;
    virtual auto ReplaceScript() noexcept -> bool = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;

    ~Input() override = default;
};
}  // namespace opentxs::blockchain::bitcoin::block::internal
