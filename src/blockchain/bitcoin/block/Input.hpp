// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/intrusive/detail/iterator.hpp>

#pragma once

#include <BlockchainTransactionInput.pb.h>
#include <BlockchainTransactionOutput.pb.h>
#include <BlockchainWalletKey.pb.h>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <cs_plain_guarded.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/bitcoin/block/Input.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/bitcoin/block/Script.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"

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

namespace proto
{
class BlockchainTransactionInput;
class BlockchainTransactionOutput;
}  // namespace proto

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::implementation
{
class Input final : public internal::Input
{
public:
    using PubkeyHashes = boost::container::flat_set<ElementHash>;

    static const VersionNumber default_version_;

    auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        UnallocatedVector<identifier::Nym>& output) const noexcept
        -> void final;
    auto AssociatedRemoteContacts(
        const api::session::Client& api,
        UnallocatedVector<identifier::Generic>& output) const noexcept
        -> void final;
    auto CalculateSize(const bool normalized) const noexcept
        -> std::size_t final;
    auto Coinbase() const noexcept -> Space final { return coinbase_; }
    auto clone() const noexcept -> std::unique_ptr<internal::Input> final
    {
        return std::make_unique<Input>(*this);
    }
    auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void final;
    auto ExtractElements(const cfilter::Type style, alloc::Default alloc)
        const noexcept -> Elements;
    auto FindMatches(
        const api::Session& api,
        const Txid& txid,
        const cfilter::Type type,
        const Patterns& txos,
        const ParsedPatterns& elements,
        const std::size_t position,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void final;
    auto GetBytes(std::size_t& base, std::size_t& witness) const noexcept
        -> void final;
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final
    {
        return cache_.keys();
    }
    auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void final;
    auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const std::size_t index,
        const Log& log) const noexcept -> opentxs::Amount final;
    auto Payer() const noexcept -> identifier::Generic
    {
        return cache_.payer();
    }
    auto PreviousOutput() const noexcept -> const Outpoint& final
    {
        return previous_;
    }
    auto Print() const noexcept -> UnallocatedCString final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto SerializeNormalized(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(
        const api::Session& api,
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool final;
    auto SetKeyData(const KeyData& data) noexcept -> void final
    {
        return cache_.set(data);
    }
    auto SignatureVersion() const noexcept
        -> std::unique_ptr<internal::Input> final;
    auto SignatureVersion(std::unique_ptr<internal::Script> subscript)
        const noexcept -> std::unique_ptr<internal::Input> final;
    auto Script() const noexcept -> const block::Script& final
    {
        return *script_;
    }
    auto Sequence() const noexcept -> std::uint32_t final { return sequence_; }
    auto Spends() const noexcept(false) -> const internal::Output& final
    {
        return cache_.spends();
    }
    auto Witness() const noexcept -> const UnallocatedVector<Space>& final
    {
        return witness_;
    }

    auto AddMultisigSignatures(const Signatures& signatures) noexcept
        -> bool final;
    auto AddSignatures(const Signatures& signatures) noexcept -> bool final;
    auto AssociatePreviousOutput(const internal::Output& output) noexcept
        -> bool final;
    auto MergeMetadata(
        const internal::Input& rhs,
        const std::size_t index,
        const Log& log) noexcept -> bool final;
    auto ReplaceScript() noexcept -> bool final;

    Input(
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        UnallocatedVector<Space>&& witness,
        std::unique_ptr<const block::Script> script,
        const VersionNumber version,
        std::optional<std::size_t> size) noexcept(false);
    Input(
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        UnallocatedVector<Space>&& witness,
        std::unique_ptr<const block::Script> script,
        const VersionNumber version,
        std::unique_ptr<internal::Output> output,
        boost::container::flat_set<crypto::Key>&& keys) noexcept(false);
    Input(
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        UnallocatedVector<Space>&& witness,
        const ReadView coinbase,
        const VersionNumber version,
        std::unique_ptr<internal::Output> output,
        std::optional<std::size_t> size = {}) noexcept(false);
    Input(
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        UnallocatedVector<Space>&& witness,
        std::unique_ptr<const block::Script> script,
        Space&& coinbase,
        const VersionNumber version,
        std::optional<std::size_t> size,
        boost::container::flat_set<crypto::Key>&& keys,
        std::unique_ptr<internal::Output> output) noexcept(false);
    Input() = delete;
    Input(const Input&) noexcept;
    Input(
        const Input& rhs,
        std::unique_ptr<const block::Script> script) noexcept;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input&&) -> Input& = delete;

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
        auto keys() const noexcept -> UnallocatedVector<crypto::Key>;
        auto net_balance_change(
            const api::crypto::Blockchain& crypto,
            const identifier::Nym& nym,
            const std::size_t index,
            const Log& log) const noexcept -> opentxs::Amount;
        auto payer() const noexcept -> identifier::Generic;
        auto spends() const noexcept(false) -> const internal::Output&;

        auto add(crypto::Key&& key) noexcept -> void;
        auto associate(const internal::Output& in) noexcept -> bool;
        auto merge(
            const internal::Input& rhs,
            const std::size_t index,
            const Log& log) noexcept -> bool;
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
            std::unique_ptr<internal::Output>&& output,
            std::optional<std::size_t>&& size,
            boost::container::flat_set<crypto::Key>&& keys) noexcept;
        Cache() = delete;
        Cache(const Cache& rhs) noexcept;

    private:
        mutable std::recursive_mutex lock_;
        std::unique_ptr<internal::Output> previous_output_;
        std::optional<std::size_t> size_;
        std::optional<std::size_t> normalized_size_;
        boost::container::flat_set<crypto::Key> keys_;
        identifier::Generic payer_;
    };

    using PubkeyMap =
        Map<int, std::pair<PubkeyHashes, std::optional<ElementHash>>>;
    using GuardedData = libguarded::plain_guarded<PubkeyMap>;

    static const VersionNumber outpoint_version_;
    static const VersionNumber key_version_;

    enum class Redeem : std::uint8_t {
        None,
        MaybeP2WSH,
        P2SH_P2WSH,
        P2SH_P2WPKH,
    };

    const blockchain::Type chain_;
    const VersionNumber serialize_version_;
    const Outpoint previous_;
    const UnallocatedVector<Space> witness_;
    const std::unique_ptr<const block::Script> script_;
    const Space coinbase_;
    const std::uint32_t sequence_;
    mutable Cache cache_;
    mutable GuardedData guarded_;

    auto classify() const noexcept -> Redeem;
    auto decode_coinbase() const noexcept -> UnallocatedCString;
    auto get_pubkeys(const api::Session& api, alloc::Default monotonic)
        const noexcept -> const PubkeyHashes&;
    auto get_script_hash(const api::Session& api) const noexcept
        -> const std::optional<ElementHash>&;
    auto index_elements(
        const api::Session& api,
        PubkeyHashes& hashes,
        alloc::Default monotonic) const noexcept -> void;
    auto is_bip16() const noexcept;
    auto payload_bytes() const noexcept -> std::size_t;
    auto serialize(const AllocateOutput destination, const bool normalized)
        const noexcept -> std::optional<std::size_t>;
};
}  // namespace opentxs::blockchain::bitcoin::block::implementation
