// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain
// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/HD.hpp"

#pragma once

#include <Bip47Channel.pb.h>
#include <HDPath.pb.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string_view>

#include "blockchain/crypto/Deterministic.hpp"
#include "blockchain/crypto/Element.hpp"
#include "blockchain/crypto/Subaccount.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Latest.hpp"

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
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block

namespace crypto
{
class Account;
class PaymentCode;
}  // namespace crypto
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto

namespace proto
{
class Bip47Channel;
class HDPath;
}  // namespace proto

class Data;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class PaymentCode final : public internal::PaymentCode, public Deterministic
{
public:
    using Element = implementation::Element;
    using SerializedType = proto::Bip47Channel;

    auto AddNotification(const block::TransactionHash& tx) const noexcept
        -> bool final;
    auto InternalPaymentCode() const noexcept -> internal::PaymentCode& final
    {
        return const_cast<PaymentCode&>(*this);
    }
    auto IsNotified() const noexcept -> bool final;
    auto Local() const noexcept -> const opentxs::PaymentCode& final
    {
        return local_;
    }
    auto ReorgNotification(const block::TransactionHash& tx) const noexcept
        -> bool final;
    auto Remote() const noexcept -> const opentxs::PaymentCode& final
    {
        return remote_;
    }
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::crypto::asymmetric::key::EllipticCurve final;
    using Deterministic::Reserve;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label,
        const Time time) const noexcept -> Batch final;
    auto RootNode(const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::HD& final;

    PaymentCode(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Account& parent,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::TransactionHash& txid,
        const PasswordPrompt& reason,
        identifier::Generic& id) noexcept(false);
    PaymentCode(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Account& parent,
        const SerializedType& serialized,
        identifier::Generic& id,
        identifier::Generic&& contact) noexcept(false);
    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    auto operator=(const PaymentCode&) -> PaymentCode& = delete;
    auto operator=(PaymentCode&&) -> PaymentCode& = delete;

    ~PaymentCode() final;

private:
    static constexpr auto internal_type_{Subchain::Outgoing};
    static constexpr auto external_type_{Subchain::Incoming};
    static constexpr auto DefaultVersion = VersionNumber{1};
    static constexpr auto Bip47DirectionVersion = VersionNumber{1};
    static constexpr auto compare_ = [](const opentxs::PaymentCode& lhs,
                                        const opentxs::PaymentCode& rhs) {
        return lhs.ID() == rhs.ID();
    };

    using Compare = std::function<
        void(const opentxs::PaymentCode&, const opentxs::PaymentCode&)>;
    using Latest =
        LatestVersion<opentxs::PaymentCode, opentxs::PaymentCode, Compare>;

    VersionNumber version_;
    mutable UnallocatedSet<opentxs::blockchain::block::TransactionHash>
        outgoing_notifications_;
    mutable UnallocatedSet<opentxs::blockchain::block::TransactionHash>
        incoming_notifications_;
    mutable Latest local_;
    Latest remote_;
    const identifier::Generic contact_id_;

    auto account_already_exists(const rLock& lock) const noexcept -> bool final;
    auto get_contact() const noexcept -> identifier::Generic final
    {
        return contact_id_;
    }
    auto has_private(const PasswordPrompt& reason) const noexcept -> bool;
    auto save(const rLock& lock) const noexcept -> bool final;
    auto set_deterministic_contact(
        UnallocatedSet<identifier::Generic>& contacts) const noexcept
        -> void final
    {
        contacts.emplace(get_contact());
    }
};
}  // namespace opentxs::blockchain::crypto::implementation
