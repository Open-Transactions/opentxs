// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainDeterministicAccountData.pb.h>
#include <HDPath.pb.h>
#include <boost/container/flat_map.hpp>
#include <cstddef>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/crypto/subaccount/base/Imp.hpp"
#include "internal/blockchain/crypto/Deterministic.hpp"
#include "internal/blockchain/crypto/Element.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
namespace implementation
{
class Element;
}  // namespace implementation

class Account;
}  // namespace crypto
}  // namespace blockchain

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class DeterministicPrivate : virtual public internal::Deterministic,
                             public SubaccountPrivate
{
public:
    auto AllowedSubchains() const noexcept -> Set<Subchain> final;
    auto asDeterministic() const noexcept
        -> const internal::Deterministic& final
    {
        return *this;
    }
    auto Floor(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto BalanceElement(const Subchain type, const Bip32Index index) const
        noexcept(false) -> const crypto::Element& final;
    auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index> final;
    auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& final;
    auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto Lookahead() const noexcept -> std::size_t final { return window_; }
    using SubaccountPrivate::PrivateKey;
    auto PrivateKey(
        const implementation::Element& element,
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& final;
    virtual auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve = 0;
    auto Path() const noexcept -> proto::HDPath final { return path_; }
    auto PathRoot() const noexcept -> const opentxs::crypto::SeedID& final;
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

    auto asDeterministic() noexcept -> internal::Deterministic& final
    {
        return *this;
    }
    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void final;

    DeterministicPrivate() = delete;
    DeterministicPrivate(const DeterministicPrivate&) = delete;
    DeterministicPrivate(DeterministicPrivate&&) = delete;
    auto operator=(const DeterministicPrivate&)
        -> DeterministicPrivate& = delete;
    auto operator=(DeterministicPrivate&&) -> DeterministicPrivate& = delete;

    ~DeterministicPrivate() override = default;

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
    const opentxs::crypto::SeedID seed_id_;
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
        return const_cast<DeterministicPrivate*>(this)->element(
            lock, type, index);
    }
    virtual auto get_contact() const noexcept -> identifier::Generic;
    auto is_generated(const rLock&, const Subchain type, Bip32Index index)
        const noexcept
    {
        return index < generated_.at(type);
    }
    auto need_lookahead(const rLock& lock, const Subchain type) const noexcept
        -> Bip32Index;
    auto reserve(
        const Subchain type,
        const std::size_t batch,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label,
        const Time time) const noexcept -> Batch;
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
    auto init(bool existing) noexcept(false) -> void final;
    auto init(const PasswordPrompt& reason) noexcept(false) -> void;

    DeterministicPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        const identifier::Account& id,
        identifier::Generic source,
        std::string_view sourceName,
        std::string_view name,
        proto::HDPath path,
        ChainData&& data) noexcept;
    DeterministicPrivate(
        const api::Session& api,
        const crypto::Account& parent,
        const SubaccountType type,
        const identifier::Account& id,
        const SerializedType& serialized,
        identifier::Generic source,
        std::string_view sourceName,
        std::string_view name,
        Bip32Index internal,
        Bip32Index external,
        ChainData&& data) noexcept(false);

private:
    using Status = internal::Element::Availability;
    using Fallback = UnallocatedMap<Status, UnallocatedSet<Bip32Index>>;
    // TODO libguarded
    using CachedKey =
        std::pair<std::mutex, opentxs::crypto::asymmetric::key::HD>;

    static constexpr auto BlockchainDeterministicAccountDataVersion =
        VersionNumber{1};

    mutable CachedKey cached_key_;

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
        const Bip32Index index) noexcept(false) -> crypto::Element& final;
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
}  // namespace opentxs::blockchain::crypto
