// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/intrusive/detail/iterator.hpp>

#pragma once

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/BlockchainTransactionInput.pb.h"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"
#include "serialization/protobuf/BlockchainWalletKey.pb.h"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

namespace proto
{
class BlockchainTransactionInput;
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Input final : public internal::Input
{
public:
    static const VersionNumber default_version_;

    auto AssociatedLocalNyms(std::pmr::vector<OTNymID>& output) const noexcept
        -> void final;
    auto AssociatedRemoteContacts(
        std::pmr::vector<OTIdentifier>& output) const noexcept -> void final;
    auto CalculateSize(const bool normalized) const noexcept
        -> std::size_t final;
    auto Coinbase() const noexcept -> Space final { return coinbase_; }
    auto clone() const noexcept -> std::unique_ptr<internal::Input> final
    {
        return std::make_unique<Input>(*this);
    }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::pmr::vector<Space> final;
    auto FindMatches(
        const ReadView txid,
        const filter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches final;
    auto GetPatterns() const noexcept -> std::pmr::vector<PatternID> final;
    auto Internal() const noexcept -> const internal::Input& final
    {
        return *this;
    }
    auto Keys() const noexcept -> std::pmr::vector<crypto::Key> final
    {
        return cache_.keys();
    }
    auto NetBalanceChange(const identifier::Nym& nym) const noexcept
        -> opentxs::Amount final
    {
        return cache_.net_balance_change(nym);
    }
    auto Payer() const noexcept -> OTIdentifier { return cache_.payer(); }
    auto PreviousOutput() const noexcept -> const Outpoint& final
    {
        return previous_;
    }
    auto Print() const noexcept -> std::string final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto SerializeNormalized(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(const std::uint32_t index, SerializeType& destination)
        const noexcept -> bool final;
    auto SetKeyData(const KeyData& data) noexcept -> void final
    {
        return cache_.set(data);
    }
    auto SignatureVersion() const noexcept
        -> std::unique_ptr<internal::Input> final;
    auto SignatureVersion(std::unique_ptr<internal::Script> subscript)
        const noexcept -> std::unique_ptr<internal::Input> final;
    auto Script() const noexcept -> const bitcoin::Script& final
    {
        return *script_;
    }
    auto Sequence() const noexcept -> std::uint32_t final { return sequence_; }
    auto Spends() const noexcept(false) -> const internal::Output& final
    {
        return cache_.spends();
    }
    auto Witness() const noexcept -> const std::pmr::vector<Space>& final
    {
        return witness_;
    }

    auto AddMultisigSignatures(const Signatures& signatures) noexcept
        -> bool final;
    auto AddSignatures(const Signatures& signatures) noexcept -> bool final;
    auto AssociatePreviousOutput(const internal::Output& output) noexcept
        -> bool final;
    auto Internal() noexcept -> internal::Input& final { return *this; }
    auto MergeMetadata(const internal::Input& rhs) noexcept -> bool final;
    auto ReplaceScript() noexcept -> bool final;

    Input(
        const api::Session& api,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::pmr::vector<Space>&& witness,
        std::unique_ptr<const internal::Script> script,
        const VersionNumber version,
        std::optional<std::size_t> size) noexcept(false);
    Input(
        const api::Session& api,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::pmr::vector<Space>&& witness,
        std::unique_ptr<const internal::Script> script,
        const VersionNumber version,
        std::unique_ptr<const internal::Output> output,
        boost::container::flat_set<crypto::Key>&& keys) noexcept(false);
    Input(
        const api::Session& api,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::pmr::vector<Space>&& witness,
        const ReadView coinbase,
        const VersionNumber version,
        std::unique_ptr<const internal::Output> output,
        std::optional<std::size_t> size = {}) noexcept(false);
    Input(
        const api::Session& api,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::pmr::vector<Space>&& witness,
        std::unique_ptr<const internal::Script> script,
        Space&& coinbase,
        const VersionNumber version,
        std::optional<std::size_t> size,
        boost::container::flat_set<crypto::Key>&& keys,
        boost::container::flat_set<PatternID>&& pubkeyHashes,
        std::optional<PatternID>&& scriptHash,
        const bool indexed,
        std::unique_ptr<const internal::Output> output) noexcept(false);
    Input(const Input&) noexcept;
    Input(
        const Input& rhs,
        std::unique_ptr<const internal::Script> script) noexcept;

    ~Input() final = default;

private:
    class Cache
    {
    public:
        template <typename F>
        auto for_each_key(F cb) const noexcept -> void
        {
            auto lock = rLock{lock_};
            std::for_each(std::begin(keys_), std::end(keys_), cb);
        }
        auto keys() const noexcept -> std::pmr::vector<crypto::Key>;
        auto net_balance_change(const identifier::Nym& nym) const noexcept
            -> opentxs::Amount;
        auto payer() const noexcept -> OTIdentifier;
        auto spends() const noexcept(false) -> const internal::Output&;

        auto add(crypto::Key&& key) noexcept -> void;
        auto associate(const internal::Output& in) noexcept -> bool;
        auto merge(const internal::Input& rhs) noexcept -> bool;
        auto reset_size() noexcept -> void;
        auto set(const KeyData& data) noexcept -> void;
        template <typename F>
        auto size(const bool normalize, F cb) noexcept -> std::size_t
        {
            auto lock = rLock{lock_};
            auto& output = normalize ? normalized_size_ : size_;

            if (false == output.has_value()) { output = cb(); }

            return output.value();
        }

        Cache(
            const api::Session& api,
            std::unique_ptr<const internal::Output>&& output,
            std::optional<std::size_t>&& size,
            boost::container::flat_set<crypto::Key>&& keys) noexcept
            : api_(api)
            , lock_()
            , previous_output_(std::move(output))
            , size_(std::move(size))
            , normalized_size_()
            , keys_(std::move(keys))
            , payer_(api_.Factory().Identifier())
        {
        }
        Cache(const Cache& rhs) noexcept
            : api_(rhs.api_)
            , lock_()
            , previous_output_()
            , size_()
            , normalized_size_()
            , keys_()
            , payer_([&] {
                auto lock = rLock{rhs.lock_};

                return rhs.payer_;
            }())
        {
            auto lock = rLock{rhs.lock_};

            if (rhs.previous_output_) {
                previous_output_ = rhs.previous_output_->clone();
            }

            size_ = rhs.size_;
            normalized_size_ = rhs.normalized_size_;
            keys_ = rhs.keys_;
        }

    private:
        const api::Session& api_;
        mutable std::recursive_mutex lock_;
        std::unique_ptr<const internal::Output> previous_output_;
        std::optional<std::size_t> size_;
        std::optional<std::size_t> normalized_size_;
        boost::container::flat_set<crypto::Key> keys_;
        OTIdentifier payer_;

        Cache() = delete;
    };

    static const VersionNumber outpoint_version_;
    static const VersionNumber key_version_;

    enum class Redeem : std::uint8_t {
        None,
        MaybeP2WSH,
        P2SH_P2WSH,
        P2SH_P2WPKH,
    };

    const api::Session& api_;
    const blockchain::Type chain_;
    const VersionNumber serialize_version_;
    const Outpoint previous_;
    const std::pmr::vector<Space> witness_;
    const std::unique_ptr<const internal::Script> script_;
    const Space coinbase_;
    const std::uint32_t sequence_;
    const boost::container::flat_set<PatternID> pubkey_hashes_;
    const std::optional<PatternID> script_hash_;
    mutable Cache cache_;

    auto classify() const noexcept -> Redeem;
    auto decode_coinbase() const noexcept -> std::string;
    auto is_bip16() const noexcept;
    auto serialize(const AllocateOutput destination, const bool normalized)
        const noexcept -> std::optional<std::size_t>;

    auto index_elements() noexcept -> void;

    Input() = delete;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input&&) -> Input& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
