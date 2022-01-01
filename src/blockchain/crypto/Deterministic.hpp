// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_map.hpp>
#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainDeterministicAccountData.pb.h"
#include "serialization/protobuf/HDPath.pb.h"

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
class Account;
class Element;
}  // namespace crypto
}  // namespace blockchain

namespace proto
{
class BlockchainDeterministicAccountData;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::blockchain::crypto::implementation
{
class Deterministic : virtual public internal::Deterministic, public Subaccount
{
public:
    auto AllowedSubchains() const noexcept -> std::pmr::set<Subchain> final;
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
        -> ECKey final;
    auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto Lookahead() const noexcept -> std::size_t final { return window_; }
    auto Path() const noexcept -> proto::HDPath final { return path_; }
    auto PathRoot() const noexcept -> const std::string final
    {
        return path_.root();
    }
    auto Reserve(
        const Subchain type,
        const PasswordPrompt& reason,
        const Identifier& contact,
        const std::string& label,
        const Time time) const noexcept -> std::optional<Bip32Index> final;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const PasswordPrompt& reason,
        const Identifier& contact,
        const std::string& label,
        const Time time) const noexcept -> Batch override;
    auto RootNode(const PasswordPrompt& reason) const noexcept
        -> blockchain::crypto::HDKey override;
    auto ScanProgress(Subchain type) const noexcept -> block::Position final;
    auto SetScanProgress(
        const block::Position& progress,
        Subchain type) noexcept -> void final;

    ~Deterministic() override = default;

protected:
    using IndexMap = std::pmr::map<Subchain, Bip32Index>;
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
        const noexcept(false) -> const Element&
    {
        return const_cast<Deterministic*>(this)->element(lock, type, index);
    }
    virtual auto get_contact() const noexcept -> OTIdentifier;
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
        const Identifier& contact,
        const std::string& label,
        const Time time,
        Batch& generated) const noexcept -> std::optional<Bip32Index>;

    auto element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false) -> crypto::Element&;
    using Subaccount::init;
    auto init(const PasswordPrompt& reason) noexcept(false) -> void;

    Deterministic(
        const api::Session& api,
        const Account& parent,
        const SubaccountType type,
        OTIdentifier&& id,
        const proto::HDPath path,
        ChainData&& data,
        Identifier& out) noexcept;
    Deterministic(
        const api::Session& api,
        const Account& parent,
        const SubaccountType type,
        const SerializedType& serialized,
        const Bip32Index generated,
        const Bip32Index used,
        ChainData&& data,
        Identifier& out) noexcept(false);

private:
    using Status = internal::Element::Availability;
    using Fallback = std::pmr::map<Status, std::pmr::set<Bip32Index>>;
    using CachedKey = std::pair<std::mutex, blockchain::crypto::HDKey>;

    static constexpr auto BlockchainDeterministicAccountDataVersion =
        VersionNumber{1};

    mutable CachedKey cached_key_;

    static auto extract_contacts(
        const Bip32Index index,
        const AddressMap& map,
        std::pmr::set<OTIdentifier>& contacts) noexcept -> void;

    auto accept(
        const rLock& lock,
        const Subchain type,
        const Identifier& contact,
        const std::string& label,
        const Time time,
        const Bip32Index index,
        Batch& generated,
        const PasswordPrompt& reason) const noexcept
        -> std::optional<Bip32Index>;
    auto check(
        const rLock& lock,
        const Subchain type,
        const Identifier& contact,
        const std::string& label,
        const Time time,
        const Bip32Index index,
        const PasswordPrompt& reason,
        Fallback& fallback,
        std::size_t& gap,
        Batch& generated) const noexcept(false) -> std::optional<Bip32Index>;
    auto check_activity(
        const rLock& lock,
        const std::pmr::vector<Activity>& unspent,
        std::pmr::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto check_lookahead(
        const rLock& lock,
        Batch& generated,
        const PasswordPrompt& reason) const noexcept(false) -> void;
    auto confirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void final;
    [[nodiscard]] auto finish_allocation(const rLock& lock, Batch& generated)
        const noexcept -> bool;
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
        std::pmr::set<OTIdentifier>&) const noexcept -> void
    {
    }
    auto set_metadata(
        const rLock& lock,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contact,
        const std::string& label) const noexcept -> void;
    auto unconfirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void final;

    Deterministic() = delete;
    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;
};
}  // namespace opentxs::blockchain::crypto::implementation
