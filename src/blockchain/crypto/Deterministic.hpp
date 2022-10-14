// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainDeterministicAccountData.pb.h>
#include <HDPath.pb.h>
#include <boost/container/flat_map.hpp>
#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace crypto
{
class Account;
class Element;
}  // namespace crypto
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class HD;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto

namespace proto
{
class BlockchainDeterministicAccountData;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Deterministic : virtual public internal::Deterministic, public Subaccount
{
public:
    auto AllowedSubchains() const noexcept -> UnallocatedSet<Subchain> final;
    auto Floor(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto BalanceElement(const Subchain type, const Bip32Index index) const
        noexcept(false) -> const crypto::Element& final;
    auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index> final;
    auto InternalDeterministic() const noexcept
        -> internal::Deterministic& final
    {
        return const_cast<Deterministic&>(*this);
    }
    auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& final;
    auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto Lookahead() const noexcept -> std::size_t final { return window_; }
    auto Path() const noexcept -> proto::HDPath final { return path_; }
    auto PathRoot() const noexcept -> const UnallocatedCString final
    {
        return path_.root();
    }
    auto Reserve(
        const Subchain type,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label,
        const Time time) const noexcept -> std::optional<Bip32Index> final;
    auto Reserve(
        const Subchain type,
        const PasswordPrompt& reason,
        const std::string_view label,
        const Time time) const noexcept -> std::optional<Bip32Index> final;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label,
        const Time time) const noexcept -> Batch override;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const PasswordPrompt& reason,
        const std::string_view label,
        const Time time) const noexcept -> Batch override;
    auto RootNode(const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::HD& override;
    auto ScanProgress(Subchain type) const noexcept -> block::Position final;

    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void final;

    Deterministic() = delete;
    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;

    ~Deterministic() override = default;

protected:
    using IndexMap = UnallocatedMap<Subchain, Bip32Index>;
    using SerializedType = proto::BlockchainDeterministicAccountData;

    struct ChainData {
        AddressData internal_;
        AddressData external_;

        auto Get(Subchain type) const noexcept(false) -> const AddressData&
        {
            return const_cast<ChainData&>(*this).Get(type);
        }

        auto Get(Subchain type) noexcept(false) -> AddressData&;

        ChainData(
            const api::Session& api,
            Subchain internalType,
            bool internalContact,
            Subchain externalType,
            bool externalContact) noexcept;
    };

    static constexpr Bip32Index window_{20u};
    static constexpr Bip32Index max_allocation_{2000u};
    static constexpr Bip32Index max_index_{2147483648u};

    const proto::HDPath path_;
    mutable ChainData data_;
    mutable IndexMap generated_;
    mutable IndexMap used_;
    mutable boost::container::flat_map<Subchain, std::optional<Bip32Index>>
        last_allocation_;

    auto check_lookahead(
        const rLock& lock,
        const Subchain type,
        Batch& generated,
        const PasswordPrompt& reason) const noexcept(false) -> void;
    auto element(const rLock& lock, const Subchain type, const Bip32Index index)
        const noexcept(false) -> const crypto::Element&
    {
        return const_cast<Deterministic*>(this)->element(lock, type, index);
    }
    virtual auto get_contact() const noexcept -> identifier::Generic;
    auto is_generated(const rLock&, const Subchain type, Bip32Index index)
        const noexcept
    {
        return index < generated_.at(type);
    }
    auto need_lookahead(const rLock& lock, const Subchain type) const noexcept
        -> Bip32Index;
    auto serialize_deterministic(const rLock& lock, SerializedType& out)
        const noexcept -> void;
    auto use_next(
        const rLock& lock,
        const Subchain type,
        const PasswordPrompt& reason,
        const identifier::Generic& contact,
        const std::string_view label,
        const Time time,
        Batch& generated) const noexcept -> std::optional<Bip32Index>;

    auto element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) -> crypto::Element&;
    auto init() noexcept -> void final;
    auto init(const PasswordPrompt& reason) noexcept(false) -> void;

    Deterministic(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        identifier::Generic&& id,
        const proto::HDPath path,
        ChainData&& data,
        identifier::Generic& out) noexcept;
    Deterministic(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        const SerializedType& serialized,
        const Bip32Index internal,
        const Bip32Index external,
        ChainData&& data,
        identifier::Generic& out) noexcept(false);

private:
    using Status = internal::Element::Availability;
    using Fallback = UnallocatedMap<Status, UnallocatedSet<Bip32Index>>;
    // TODO libguarded
    using CachedKey =
        std::pair<std::mutex, opentxs::crypto::asymmetric::key::HD>;

    static constexpr auto BlockchainDeterministicAccountDataVersion =
        VersionNumber{1};

    mutable CachedKey cached_key_;

    static auto extract_contacts(
        const Bip32Index index,
        const AddressMap& map,
        UnallocatedSet<identifier::Generic>& contacts) noexcept -> void;

    auto accept(
        const rLock& lock,
        const Subchain type,
        const identifier::Generic& contact,
        const std::string_view label,
        const Time time,
        const Bip32Index index,
        Batch& generated,
        const PasswordPrompt& reason) const noexcept
        -> std::optional<Bip32Index>;
    auto check(
        const rLock& lock,
        const Subchain type,
        const identifier::Generic& contact,
        const std::string_view label,
        const Time time,
        const Bip32Index index,
        const PasswordPrompt& reason,
        Fallback& fallback,
        std::size_t& gap,
        Batch& generated) const noexcept(false) -> std::optional<Bip32Index>;
    auto check_activity(
        const rLock& lock,
        const UnallocatedVector<Activity>& unspent,
        UnallocatedSet<identifier::Generic>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto check_lookahead(
        const rLock& lock,
        Batch& internal,
        Batch& external,
        const PasswordPrompt& reason) const noexcept(false) -> void;
    auto confirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void final;
    [[nodiscard]] auto finish_allocation(
        const rLock& lock,
        const Subchain type,
        const Batch& batch) const noexcept -> bool;
    [[nodiscard]] auto finish_allocation(
        const rLock& lock,
        const Batch& internal,
        const Batch& external) const noexcept -> bool;
    [[nodiscard]] auto generate(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept(false) -> Bip32Index;
    [[nodiscard]] auto generate_next(
        const rLock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false) -> Bip32Index;
    auto mutable_element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) -> Element& final;
    virtual auto set_deterministic_contact(
        UnallocatedSet<identifier::Generic>&) const noexcept -> void
    {
    }
    auto set_metadata(
        const rLock& lock,
        const Subchain subchain,
        const Bip32Index index,
        const identifier::Generic& contact,
        const std::string_view label) const noexcept -> void;
    auto unconfirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void final;
};
}  // namespace opentxs::blockchain::crypto::implementation
