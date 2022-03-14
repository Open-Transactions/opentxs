// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>

#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace bitcoin
{
struct SigHash;
}  // namespace bitcoin
}  // namespace blockchain

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Transaction final : public internal::Transaction
{
public:
    static const VersionNumber default_version_;

    auto AssociatedLocalNyms() const noexcept
        -> UnallocatedVector<OTNymID> final;
    auto AssociatedRemoteContacts(
        const api::session::Contacts& contacts,
        const identifier::Nym& nym) const noexcept
        -> UnallocatedVector<OTIdentifier> final;
    auto BlockPosition() const noexcept -> std::optional<std::size_t> final
    {
        return position_;
    }
    auto CalculateSize() const noexcept -> std::size_t final
    {
        return calculate_size(false);
    }
    auto Chains() const noexcept -> UnallocatedVector<blockchain::Type> final
    {
        return cache_.chains();
    }
    auto clone() const noexcept -> std::unique_ptr<bitcoin::Transaction> final
    {
        return std::make_unique<Transaction>(*this);
    }
    auto ConfirmationHeight() const noexcept -> block::Height final
    {
        return cache_.height();
    }
    auto ExtractElements(const cfilter::Type style) const noexcept
        -> UnallocatedVector<Space> final;
    auto FindMatches(
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches final;
    auto GetPatterns() const noexcept -> UnallocatedVector<PatternID> final;
    auto GetPreimageBTC(
        const std::size_t index,
        const blockchain::bitcoin::SigHash& hashType) const noexcept
        -> Space final;
    auto ID() const noexcept -> const Txid& final { return txid_; }
    auto IDNormalized() const noexcept -> const Identifier& final;
    auto Inputs() const noexcept -> const bitcoin::Inputs& final
    {
        return *inputs_;
    }
    auto Internal() const noexcept -> const internal::Transaction& final
    {
        return *this;
    }
    auto IsGeneration() const noexcept -> bool final { return is_generation_; }
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final;
    auto Locktime() const noexcept -> std::uint32_t final { return lock_time_; }
    auto Memo() const noexcept -> UnallocatedCString final;
    auto MinedPosition() const noexcept -> const block::Position& final
    {
        return cache_.position();
    }
    auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount final;
    auto Outputs() const noexcept -> const bitcoin::Outputs& final
    {
        return *outputs_;
    }
    auto SegwitFlag() const noexcept -> std::byte final { return segwit_flag_; }
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize() const noexcept -> std::optional<SerializeType> final;
    auto Timestamp() const noexcept -> Time final { return time_; }
    auto Version() const noexcept -> std::int32_t final { return version_; }
    auto vBytes(blockchain::Type chain) const noexcept -> std::size_t final;
    auto WTXID() const noexcept -> const Txid& final { return wtxid_; }

    auto AssociatePreviousOutput(
        const std::size_t index,
        const internal::Output& output) noexcept -> bool final
    {
        return inputs_->AssociatePreviousOutput(index, output);
    }
    auto ForTestingOnlyAddKey(
        const std::size_t index,
        const blockchain::crypto::Key& key) noexcept -> bool final
    {
        return outputs_->ForTestingOnlyAddKey(index, key);
    }
    auto Internal() noexcept -> internal::Transaction& final { return *this; }
    auto MergeMetadata(
        const blockchain::Type chain,
        const internal::Transaction& rhs) noexcept -> void final;
    auto Print() const noexcept -> UnallocatedCString final;
    auto SetKeyData(const KeyData& data) noexcept -> void final;
    auto SetMemo(const UnallocatedCString& memo) noexcept -> void final
    {
        cache_.set_memo(memo);
    }
    auto SetMinedPosition(const block::Position& pos) noexcept -> void final
    {
        return cache_.set_position(pos);
    }
    auto SetPosition(std::size_t position) noexcept -> void final
    {
        const_cast<std::optional<std::size_t>&>(position_) = position;
    }

    Transaction(
        const api::Session& api,
        const VersionNumber serializeVersion,
        const bool isGeneration,
        const std::int32_t version,
        const std::byte segwit,
        const std::uint32_t lockTime,
        const pTxid&& txid,
        const pTxid&& wtxid,
        const Time& time,
        const UnallocatedCString& memo,
        std::unique_ptr<internal::Inputs> inputs,
        std::unique_ptr<internal::Outputs> outputs,
        UnallocatedVector<blockchain::Type>&& chains,
        block::Position&& minedPosition,
        std::optional<std::size_t>&& position = std::nullopt) noexcept(false);
    Transaction(const Transaction&) noexcept;

    ~Transaction() final = default;

private:
    class Cache
    {
    public:
        auto chains() const noexcept -> UnallocatedVector<blockchain::Type>;
        auto height() const noexcept -> block::Height;
        auto memo() const noexcept -> UnallocatedCString;
        auto position() const noexcept -> const block::Position&;

        auto add(blockchain::Type chain) noexcept -> void;
        auto merge(const internal::Transaction& rhs) noexcept -> void;
        template <typename F>
        auto normalized(F cb) noexcept -> const Identifier&
        {
            auto lock = rLock{lock_};
            auto& output = normalized_id_;

            if (false == output.has_value()) { output = cb(); }

            return output.value();
        }
        auto reset_size() noexcept -> void;
        auto set_memo(const UnallocatedCString& memo) noexcept -> void;
        auto set_memo(UnallocatedCString&& memo) noexcept -> void;
        auto set_position(const block::Position& pos) noexcept -> void;
        template <typename F>
        auto size(const bool normalize, F cb) noexcept -> std::size_t
        {
            auto lock = rLock{lock_};

            auto& output = normalize ? normalized_size_ : size_;

            if (false == output.has_value()) { output = cb(); }

            return output.value();
        }

        Cache(
            const UnallocatedCString& memo,
            UnallocatedVector<blockchain::Type>&& chains,
            block::Position&& minedPosition) noexcept(false);
        Cache(const Cache& rhs) noexcept;

    private:
        mutable std::recursive_mutex lock_;
        std::optional<OTIdentifier> normalized_id_;
        std::optional<std::size_t> size_;
        std::optional<std::size_t> normalized_size_;
        UnallocatedCString memo_;
        UnallocatedVector<blockchain::Type> chains_;
        block::Position mined_position_;

        Cache() = delete;
        Cache(Cache&&) = delete;
    };

    const api::Session& api_;
    const std::optional<std::size_t> position_;
    const VersionNumber serialize_version_;
    const bool is_generation_;
    const std::int32_t version_;
    const std::byte segwit_flag_;
    const std::uint32_t lock_time_;
    const pTxid txid_;
    const pTxid wtxid_;
    const Time time_;
    const std::unique_ptr<internal::Inputs> inputs_;
    const std::unique_ptr<internal::Outputs> outputs_;
    mutable Cache cache_;

    static auto calculate_witness_size(const Space& witness) noexcept
        -> std::size_t;
    static auto calculate_witness_size(const UnallocatedVector<Space>&) noexcept
        -> std::size_t;

    auto base_size() const noexcept -> std::size_t;
    auto calculate_size(const bool normalize) const noexcept -> std::size_t;
    auto calculate_witness_size() const noexcept -> std::size_t;
    auto serialize(const AllocateOutput destination, const bool normalize)
        const noexcept -> std::optional<std::size_t>;

    Transaction() = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
