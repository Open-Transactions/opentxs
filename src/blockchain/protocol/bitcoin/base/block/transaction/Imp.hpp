// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

#include "blockchain/block/transaction/Imp.hpp"
#include "blockchain/block/transaction/TransactionPrivate.hpp"
#include "blockchain/protocol/bitcoin/base/block/transaction/Data.hpp"
#include "blockchain/protocol/bitcoin/base/block/transaction/TransactionPrivate.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
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
class Factory;
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal
}  // namespace block

struct EncodedTransaction;
struct SigHash;
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block::implementation
{
class Transaction final
    : virtual public blockchain::block::implementation::Transaction,
      virtual public TransactionPrivate
{
public:
    static const VersionNumber default_version_;

    auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        alloc::Default alloc) const noexcept -> Set<identifier::Nym> final;
    auto AssociatedRemoteContacts(
        const api::session::Client& api,
        const identifier::Nym& nym,
        alloc::Default alloc) const noexcept -> Set<identifier::Generic> final;
    auto BlockPosition() const noexcept -> std::optional<std::size_t> final;
    auto CalculateSize() const noexcept -> std::size_t final;
    auto Chains(allocator_type alloc) const noexcept
        -> Set<blockchain::Type> final;
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> blockchain::block::TransactionPrivate* final
    {
        return pmr::clone_as<blockchain::block::TransactionPrivate>(
            this, {alloc});
    }
    auto ConfirmationHeight() const noexcept -> block::Height final;
    auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void final;
    auto FindMatches(
        const api::Session& api,
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements,
        const Log& log,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> Matches final;
    auto FindMatches(
        const api::Session& api,
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void final;
    auto GetPreimageBTC(
        const std::size_t index,
        const blockchain::protocol::bitcoin::base::SigHash& hashType)
        const noexcept -> Space final;
    auto IDNormalized(const api::Factory& factory) const noexcept
        -> const identifier::Generic& final;
    auto IndexElements(const api::Session& api, alloc::Default alloc)
        const noexcept -> ElementHashes final;
    auto Inputs() const noexcept -> std::span<const block::Input> final
    {
        return inputs_;
    }
    auto IsGeneration() const noexcept -> bool final { return is_generation_; }
    auto IsValid() const noexcept -> bool final { return true; }
    auto Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key> final;
    auto Locktime() const noexcept -> std::uint32_t final { return lock_time_; }
    auto Memo(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString final;
    auto Memo(const api::crypto::Blockchain& crypto, alloc::Default alloc)
        const noexcept -> CString final;
    auto MinedPosition() const noexcept -> const block::Position& final;
    auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount final;
    auto Outputs() const noexcept -> std::span<const block::Output> final
    {
        return outputs_;
    }
    auto Print(const api::Crypto& crypto) const noexcept
        -> UnallocatedCString final;
    auto Print(const api::Crypto& crypto, alloc::Default alloc) const noexcept
        -> CString final;
    auto SegwitFlag() const noexcept -> std::byte final { return segwit_flag_; }
    auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(const api::Session& api) const noexcept
        -> std::optional<SerializeType> final;
    auto Serialize(EncodedTransaction& out) const noexcept -> bool final;
    auto Timestamp() const noexcept -> Time final { return time_; }
    auto Version() const noexcept -> std::int32_t final { return version_; }
    auto vBytes(blockchain::Type chain) const noexcept -> std::size_t final;

    auto AssociatePreviousOutput(
        const std::size_t index,
        const block::Output& output) noexcept -> bool final;
    auto ConfirmMatches(
        const Log& log,
        const api::crypto::Blockchain& api,
        const Matches& candiates,
        database::BlockMatches& out,
        alloc::Strategy alloc) noexcept -> void final;
    auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool final;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    auto MergeMetadata(
        const api::crypto::Blockchain& crypto,
        const blockchain::Type chain,
        const internal::Transaction& rhs,
        const Log& log) noexcept -> void final;
    auto RefreshContacts(const api::crypto::Blockchain& api) noexcept
        -> void final;
    auto SetKeyData(const KeyData& data) noexcept -> void final;
    auto SetMemo(const std::string_view memo) noexcept -> void final;
    auto SetMinedPosition(const block::Position& pos) noexcept -> void final;
    auto SetPosition(std::size_t position) noexcept -> void final;

    Transaction(
        const VersionNumber serializeVersion,
        const bool isGeneration,
        const std::int32_t version,
        const std::byte segwit,
        const std::uint32_t lockTime,
        const TransactionHash& txid,
        const TransactionHash& wtxid,
        const Time& time,
        std::string_view memo,
        Vector<blockchain::protocol::bitcoin::base::block::Input> inputs,
        Vector<blockchain::protocol::bitcoin::base::block::Output> outputs,
        std::optional<ByteArray> dip2,
        Set<blockchain::Type> chains,
        block::Position&& minedPosition,
        std::optional<std::size_t>&& position,
        allocator_type alloc) noexcept(false);
    Transaction() = delete;
    Transaction(const Transaction& rhs, allocator_type alloc) noexcept;
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;

    ~Transaction() final;

private:
    const std::optional<std::size_t> position_;
    const VersionNumber serialize_version_;
    const bool is_generation_;
    const std::int32_t version_;
    const std::byte segwit_flag_;
    const std::uint32_t lock_time_;
    const Time time_;
    Vector<block::Input> inputs_;
    Vector<block::Output> outputs_;
    std::optional<ByteArray> dip_2_;
    mutable libguarded::plain_guarded<transaction::Data> data_;

    static auto calculate_witness_size(const WitnessItem& witness) noexcept
        -> std::size_t;
    static auto calculate_witness_size(
        std::span<const WitnessItem> witnesses) noexcept -> std::size_t;
    static auto calculate_witness_sizes(
        std::span<const WitnessItem> in) noexcept -> std::size_t;

    auto base_size() const noexcept -> std::size_t;
    auto calculate_dip2_size() const noexcept -> std::size_t;
    auto calculate_input_size(const bool normalize) const noexcept
        -> std::size_t;
    auto calculate_input_sizes(const bool normalize) const noexcept
        -> std::size_t;
    auto calculate_output_size() const noexcept -> std::size_t;
    auto calculate_output_sizes() const noexcept -> std::size_t;
    auto calculate_size(const bool normalize, transaction::Data& data)
        const noexcept -> std::size_t;
    auto calculate_witness_size() const noexcept -> std::size_t;
    auto calculate_witness_sizes() const noexcept -> std::size_t;
    auto serialize(
        Writer&& destination,
        const bool normalize,
        transaction::Data& data) const noexcept -> std::optional<std::size_t>;

    auto merge_metadata(
        const api::crypto::Blockchain& crypto,
        std::span<const block::Input> rhs,
        const Log& log) noexcept -> void;
    auto merge_metadata(
        const api::crypto::Blockchain& crypto,
        std::span<const block::Output> rhs,
        const Log& log) noexcept -> void;
};
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::base::block::implementation
