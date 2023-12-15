// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <algorithm>
// IWYU pragma: no_include <boost/intrusive/detail/iterator.hpp>

#pragma once

#include <cs_plain_guarded.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/output/OutputPrivate.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/bitcoincash/token/Types.hpp"  // IWYU pragma: keep
#include "internal/util/Mutex.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/bitcoincash/token/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
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
class Output;
}  // namespace internal
}  // namespace block
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
class Output final : public OutputPrivate
{
public:
    using PubkeyHashes = Set<ElementHash>;

    static const VersionNumber default_version_;
    static const VersionNumber key_version_;

    auto AssociatedLocalNyms(
        const api::crypto::Blockchain& crypto,
        Set<identifier::Nym>& output) const noexcept -> void final;
    auto AssociatedRemoteContacts(
        const api::session::Client& api,
        Set<identifier::Generic>& output) const noexcept -> void final;
    auto CalculateSize() const noexcept -> std::size_t final;
    auto Cashtoken() const noexcept
        -> const bitcoincash::token::cashtoken::View* final;
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> OutputPrivate* final
    {
        return pmr::clone_as<OutputPrivate>(this, {alloc});
    }
    auto ExtractElements(const cfilter::Type style, Elements& out)
        const noexcept -> void final;
    auto ExtractElements(const cfilter::Type style, alloc::Default alloc)
        const noexcept -> Elements;
    auto FindMatches(
        const api::Session& api,
        const TransactionHash& txid,
        const cfilter::Type type,
        const ParsedPatterns& patterns,
        const Log& log,
        Matches& out,
        alloc::Default monotonic) const noexcept -> void final;
    auto HasKeys() const noexcept -> bool final { return cache_.has_keys(); }
    auto IndexElements(const api::Session& api, ElementHashes& out)
        const noexcept -> void final;
    auto IsValid() const noexcept -> bool final { return true; }
    auto Keys(alloc::Default alloc) const noexcept -> Set<crypto::Key> final;
    auto Keys(Set<crypto::Key>& out) const noexcept -> void final;
    auto MinedPosition() const noexcept -> const block::Position& final
    {
        return cache_.position();
    }
    auto NetBalanceChange(
        const api::crypto::Blockchain& crypto,
        const identifier::Nym& nym,
        const Log& log) const noexcept -> opentxs::Amount final;
    auto Note(const api::crypto::Blockchain& crypto) const noexcept
        -> UnallocatedCString final;
    auto Note(const api::crypto::Blockchain& crypto, alloc::Default alloc)
        const noexcept -> CString final;
    auto Payee() const noexcept -> ContactID final { return cache_.payee(); }
    auto Payer() const noexcept -> ContactID final { return cache_.payer(); }
    auto Print(const api::Crypto& api) const noexcept
        -> UnallocatedCString final;
    auto Print(const api::Crypto& api, alloc::Default alloc) const noexcept
        -> CString final;
    auto Serialize(Writer&& destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(const api::Session& api, SerializeType& destination)
        const noexcept -> bool final;
    auto SigningSubscript(alloc::Default alloc) const noexcept
        -> block::Script final;
    auto Script() const noexcept -> const block::Script& final;
    auto State() const noexcept -> node::TxoState final
    {
        return cache_.state();
    }
    auto Tags() const noexcept -> const UnallocatedSet<node::TxoTag> final
    {
        return cache_.tags();
    }
    auto Value() const noexcept -> Amount final { return value_; }

    auto AddTag(node::TxoTag tag) noexcept -> void final { cache_.add(tag); }
    auto ConfirmMatches(
        const Log& log,
        const api::crypto::Blockchain& api,
        const Matches& candiates) noexcept -> bool final;
    auto ForTestingOnlyAddKey(const crypto::Key& key) noexcept -> void final
    {
        cache_.add(crypto::Key{key});
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto MergeMetadata(
        const api::Crypto& crypto,
        const internal::Output& rhs,
        const Log& log) noexcept -> void final;
    auto RefreshContacts(const api::crypto::Blockchain& api) noexcept
        -> void final;
    auto SetIndex(const std::uint32_t index) noexcept -> void final
    {
        const_cast<std::uint32_t&>(index_) = index;
    }
    auto SetKeyData(const KeyData& data) noexcept -> void final
    {
        cache_.set(data);
    }
    auto SetMinedPosition(const block::Position& pos) noexcept -> void final
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
    auto SetValue(const Amount& value) noexcept -> void final
    {
        const_cast<Amount&>(value_) = value;
    }

    Output(
        const blockchain::Type chain,
        const std::uint32_t index,
        const Amount& value,
        const std::size_t size,
        const ReadView script,
        const VersionNumber version,
        std::optional<const bitcoincash::token::cashtoken::Value> cashtoken,
        allocator_type alloc) noexcept(false);
    Output(
        const blockchain::Type chain,
        const std::uint32_t index,
        const Amount& value,
        block::Script script,
        Set<crypto::Key>&& keys,
        const VersionNumber version,
        std::optional<const bitcoincash::token::cashtoken::Value> cashtoken,
        allocator_type alloc) noexcept(false);
    Output(
        const blockchain::Type chain,
        const VersionNumber version,
        const std::uint32_t index,
        const Amount& value,
        block::Script script,
        std::optional<std::size_t> size,
        Set<crypto::Key>&& keys,
        block::Position minedPosition,
        node::TxoState state,
        UnallocatedSet<node::TxoTag> tags,
        std::optional<const bitcoincash::token::cashtoken::Value> cashtoken,
        allocator_type alloc) noexcept(false);
    Output() = delete;
    Output(const Output&, allocator_type alloc) noexcept;
    Output(const Output&) = delete;
    Output(Output&&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;

    ~Output() final;

private:
    struct Cache {
        template <typename F>
        auto for_each_key(F cb) const noexcept -> void
        {
            auto lock = Lock{lock_};
            std::for_each(keys_.begin(), keys_.end(), cb);
        }
        auto has_keys() const noexcept -> bool;
        auto keys(Set<crypto::Key>& out) const noexcept -> void;
        auto payee() const noexcept -> identifier::Generic;
        auto payer() const noexcept -> identifier::Generic;
        auto position() const noexcept -> const block::Position&;
        auto state() const noexcept -> node::TxoState;
        auto tags() const noexcept -> UnallocatedSet<node::TxoTag>;

        auto add(crypto::Key key) noexcept -> void;
        auto add(node::TxoTag tag) noexcept -> void;
        auto merge(
            const api::Crypto& crypto,
            const internal::Output& rhs,
            const std::size_t index,
            const Log& log) noexcept -> void;
        auto reset_size() noexcept -> void;
        auto set(const KeyData& data) noexcept -> void;
        auto set_payee(const identifier::Generic& contact) noexcept -> void;
        auto set_payee(identifier::Generic&& contact) noexcept -> void;
        auto set_payer(const identifier::Generic& contact) noexcept -> void;
        auto set_payer(identifier::Generic&& contact) noexcept -> void;
        auto set_position(const block::Position& pos) noexcept -> void;
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
            std::optional<std::size_t>&& size,
            Set<crypto::Key>&& keys,
            block::Position&& minedPosition,
            node::TxoState state,
            UnallocatedSet<node::TxoTag>&& tags) noexcept;
        Cache() noexcept = delete;
        Cache(const Cache& rhs) noexcept;

    private:
        mutable std::mutex lock_{};
        std::optional<std::size_t> size_{};
        identifier::Generic payee_;
        identifier::Generic payer_;
        Set<crypto::Key> keys_;
        block::Position mined_position_;
        node::TxoState state_;
        UnallocatedSet<node::TxoTag> tags_;
    };

    using PubkeyMap =
        Map<int, std::pair<PubkeyHashes, std::optional<ElementHash>>>;
    using GuardedData = libguarded::plain_guarded<PubkeyMap>;

    const blockchain::Type chain_;
    const VersionNumber serialize_version_;
    const std::uint32_t index_;
    const Amount value_;
    const block::Script script_;
    const std::optional<const bitcoincash::token::cashtoken::Value> cashtoken_;
    const bitcoincash::token::cashtoken::View cashtoken_view_;
    mutable Cache cache_;
    mutable GuardedData guarded_;

    auto get_pubkeys(const api::Session& api, alloc::Default monotonic)
        const noexcept -> const PubkeyHashes&;
    auto get_script_hash(const api::Session& api) const noexcept
        -> const std::optional<ElementHash>&;
    auto index_elements(
        const api::Session& api,
        PubkeyHashes& hashes,
        alloc::Default monotonic) const noexcept -> void;
    auto script_bytes() const noexcept
        -> std::tuple<std::size_t, std::size_t, std::size_t>;
};
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::base::block::implementation
