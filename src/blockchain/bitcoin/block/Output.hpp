// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/intrusive/detail/iterator.hpp>
// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

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
#include <utility>

#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/bitcoin/block/Script.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

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

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::implementation
{
class Output final : public internal::Output
{
public:
    auto AssociatedLocalNyms(UnallocatedVector<identifier::Nym>& output)
        const noexcept -> void final;
    auto AssociatedRemoteContacts(
        UnallocatedVector<identifier::Generic>& output) const noexcept
        -> void final;
    auto CalculateSize() const noexcept -> std::size_t final;
    auto clone() const noexcept -> std::unique_ptr<internal::Output> final
    {
        return std::make_unique<Output>(*this);
    }
    auto ExtractElements(const cfilter::Type style) const noexcept
        -> Vector<Vector<std::byte>> final;
    auto FindMatches(
        const blockchain::block::Txid& txid,
        const cfilter::Type type,
        const blockchain::block::ParsedPatterns& patterns,
        const Log& log) const noexcept -> blockchain::block::Matches final;
    auto GetPatterns() const noexcept -> UnallocatedVector<PatternID> final;
    auto Internal() const noexcept -> const internal::Output& final
    {
        return *this;
    }
    auto Keys() const noexcept -> UnallocatedVector<crypto::Key> final
    {
        return cache_.keys();
    }
    auto MinedPosition() const noexcept
        -> const blockchain::block::Position& final
    {
        return cache_.position();
    }
    auto NetBalanceChange(const identifier::Nym& nym, const Log& log)
        const noexcept -> opentxs::Amount final;
    auto Note() const noexcept -> UnallocatedCString final;
    auto Payee() const noexcept -> ContactID final { return cache_.payee(); }
    auto Payer() const noexcept -> ContactID final { return cache_.payer(); }
    auto Print() const noexcept -> UnallocatedCString final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(SerializeType& destination) const noexcept -> bool final;
    auto SigningSubscript() const noexcept
        -> std::unique_ptr<internal::Script> final
    {
        return script_->SigningSubscript(chain_);
    }
    auto Script() const noexcept -> const internal::Script& final;
    auto State() const noexcept -> node::TxoState final
    {
        return cache_.state();
    }
    auto Tags() const noexcept -> const UnallocatedSet<node::TxoTag> final
    {
        return cache_.tags();
    }
    auto Value() const noexcept -> blockchain::Amount final { return value_; }

    auto AddTag(node::TxoTag tag) noexcept -> void final { cache_.add(tag); }
    auto ForTestingOnlyAddKey(const crypto::Key& key) noexcept -> void final
    {
        cache_.add(crypto::Key{key});
    }
    auto Internal() noexcept -> internal::Output& final { return *this; }
    auto MergeMetadata(const internal::Output& rhs, const Log& log) noexcept
        -> bool final;
    auto SetIndex(const std::uint32_t index) noexcept -> void final
    {
        const_cast<std::uint32_t&>(index_) = index;
    }
    auto SetKeyData(const blockchain::block::KeyData& data) noexcept
        -> void final
    {
        cache_.set(data);
    }
    auto SetMinedPosition(const blockchain::block::Position& pos) noexcept
        -> void final
    {
        cache_.set_position(pos);
    }
    auto SetPayee(const identifier::Generic& contact) noexcept -> void final
    {
        cache_.set_payee(contact);
    }
    auto SetPayer(const identifier::Generic& contact) noexcept -> void final
    {
        cache_.set_payer(contact);
    }
    auto SetState(node::TxoState state) noexcept -> void final
    {
        cache_.set_state(state);
    }
    auto SetValue(const blockchain::Amount& value) noexcept -> void final
    {
        const_cast<blockchain::Amount&>(value_) = value;
    }

    Output(
        const api::Session& api,
        const blockchain::Type chain,
        const std::uint32_t index,
        const blockchain::Amount& value,
        const std::size_t size,
        const ReadView script,
        const VersionNumber version = default_version_) noexcept(false);
    Output(
        const api::Session& api,
        const blockchain::Type chain,
        const std::uint32_t index,
        const blockchain::Amount& value,
        std::unique_ptr<const internal::Script> script,
        boost::container::flat_set<crypto::Key>&& keys,
        const VersionNumber version = default_version_) noexcept(false);
    Output(
        const api::Session& api,
        const blockchain::Type chain,
        const VersionNumber version,
        const std::uint32_t index,
        const blockchain::Amount& value,
        std::unique_ptr<const internal::Script> script,
        std::optional<std::size_t> size,
        boost::container::flat_set<crypto::Key>&& keys,
        boost::container::flat_set<PatternID>&& pubkeyHashes,
        std::optional<PatternID>&& scriptHash,
        bool indexed,
        blockchain::block::Position minedPosition,
        node::TxoState state,
        UnallocatedSet<node::TxoTag> tags) noexcept(false);
    Output() = delete;
    Output(const Output&) noexcept;
    Output(Output&&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;

    ~Output() final = default;

private:
    struct Cache {
        template <typename F>
        auto for_each_key(F cb) const noexcept -> void
        {
            auto lock = Lock{lock_};
            std::for_each(std::begin(keys_), std::end(keys_), cb);
        }
        auto keys() const noexcept -> UnallocatedVector<crypto::Key>;
        auto payee() const noexcept -> identifier::Generic;
        auto payer() const noexcept -> identifier::Generic;
        auto position() const noexcept -> const blockchain::block::Position&;
        auto state() const noexcept -> node::TxoState;
        auto tags() const noexcept -> UnallocatedSet<node::TxoTag>;

        auto add(crypto::Key&& key) noexcept -> void;
        auto add(node::TxoTag tag) noexcept -> void;
        auto merge(
            const internal::Output& rhs,
            const std::size_t index,
            const Log& log) noexcept -> bool;
        auto reset_size() noexcept -> void;
        auto set(const blockchain::block::KeyData& data) noexcept -> void;
        auto set_payee(const identifier::Generic& contact) noexcept -> void;
        auto set_payer(const identifier::Generic& contact) noexcept -> void;
        auto set_position(const blockchain::block::Position& pos) noexcept
            -> void;
        auto set_state(node::TxoState state) noexcept -> void;
        template <typename F>
        auto size(F cb) noexcept -> std::size_t
        {
            auto lock = Lock{lock_};

            auto& output = size_;

            if (false == output.has_value()) { output = cb(); }

            return output.value();
        }

        Cache(
            const api::Session& api,
            std::optional<std::size_t>&& size,
            boost::container::flat_set<crypto::Key>&& keys,
            blockchain::block::Position&& minedPosition,
            node::TxoState state,
            UnallocatedSet<node::TxoTag>&& tags) noexcept;
        Cache() noexcept = delete;
        Cache(const Cache& rhs) noexcept;

    private:
        mutable std::mutex lock_{};
        std::optional<std::size_t> size_{};
        identifier::Generic payee_;
        identifier::Generic payer_;
        boost::container::flat_set<crypto::Key> keys_;
        blockchain::block::Position mined_position_;
        node::TxoState state_;
        UnallocatedSet<node::TxoTag> tags_;

        auto set_payee(identifier::Generic&& contact) noexcept -> void;
        auto set_payer(identifier::Generic&& contact) noexcept -> void;
    };

    static const VersionNumber default_version_;
    static const VersionNumber key_version_;

    const api::Session& api_;
    const blockchain::Type chain_;
    const VersionNumber serialize_version_;
    const std::uint32_t index_;
    const blockchain::Amount value_;
    const std::unique_ptr<const internal::Script> script_;
    const boost::container::flat_set<PatternID> pubkey_hashes_;
    const std::optional<PatternID> script_hash_;
    mutable Cache cache_;

    auto index_elements() noexcept -> void;
};
}  // namespace opentxs::blockchain::bitcoin::block::implementation
