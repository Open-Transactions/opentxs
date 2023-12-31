// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/key/HD.hpp"

#pragma once

#include <opentxs/protobuf/Bip47Channel.pb.h>
#include <opentxs/protobuf/BlockchainAddress.pb.h>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "blockchain/crypto/element/Element.hpp"
#include "blockchain/crypto/subaccount/deterministic/Imp.hpp"
#include "internal/blockchain/crypto/Deterministic.hpp"
#include "internal/blockchain/crypto/PaymentCode.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "util/Latest.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain

namespace protobuf
{
class HDPath;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class PaymentCodePrivate final : public internal::PaymentCode,
                                 public DeterministicPrivate
{
public:
    using Element = implementation::Element;
    using SerializedType = protobuf::Bip47Channel;

    auto AddIncomingNotification(
        const block::TransactionHash& tx) const noexcept -> bool final;
    auto AddNotification(const block::TransactionHash& tx) const noexcept
        -> bool final;

    auto asDeterministicPublic() const noexcept
        -> const crypto::Deterministic& final
    {
        return const_cast<PaymentCodePrivate*>(this)->asPaymentCodePublic();
    }
    auto asPaymentCode() const noexcept -> const internal::PaymentCode& final
    {
        return *this;
    }
    auto asPaymentCodePublic() const noexcept
        -> const crypto::PaymentCode& final
    {
        return const_cast<PaymentCodePrivate*>(this)->asPaymentCodePublic();
    }
    auto IncomingNotificationCount() const noexcept -> std::size_t final;
    auto Local() const noexcept -> const opentxs::PaymentCode& final
    {
        return local_;
    }
    auto NotificationCount() const noexcept
        -> std::pair<std::size_t, std::size_t> final;
    auto OutgoingNotificationCount() const noexcept -> std::size_t final;
    using Subaccount::PrivateKey;
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve final;
    auto Remote() const noexcept -> const opentxs::PaymentCode& final
    {
        return remote_;
    }
    auto ReorgNotification(const block::TransactionHash& tx) const noexcept
        -> bool final;
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
    auto Self() const noexcept -> const crypto::Subaccount& final
    {
        return asPaymentCodePublic();
    }

    auto asDeterministicPublic() noexcept -> crypto::Deterministic& final
    {
        return asPaymentCodePublic();
    }
    auto asPaymentCode() noexcept -> internal::PaymentCode& final
    {
        return *this;
    }
    auto asPaymentCodePublic() noexcept -> crypto::PaymentCode& final
    {
        return *self_;
    }
    auto InitSelf(std::shared_ptr<Subaccount> me) noexcept -> void final;
    auto Self() noexcept -> crypto::Subaccount& final
    {
        return asPaymentCodePublic();
    }

    PaymentCodePrivate(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Account& parent,
        const identifier::Account& id,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const protobuf::HDPath& path,
        const PasswordPrompt& reason) noexcept(false);
    PaymentCodePrivate(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Account& parent,
        const identifier::Account& id,
        const SerializedType& serialized,
        identifier::Generic&& contact) noexcept(false);
    PaymentCodePrivate(const PaymentCodePrivate&) = delete;
    PaymentCodePrivate(PaymentCodePrivate&&) = delete;
    auto operator=(const PaymentCodePrivate&) -> PaymentCodePrivate& = delete;
    auto operator=(PaymentCodePrivate&&) -> PaymentCodePrivate& = delete;

    ~PaymentCodePrivate() final;

private:
    using Me = std::optional<crypto::PaymentCode>;

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
    mutable Me self_;

    static auto name(const opentxs::PaymentCode& remote) noexcept
        -> UnallocatedCString;
    static auto source(const opentxs::PaymentCode& local) noexcept
        -> UnallocatedCString;

    auto account_already_exists(const rLock& lock) const noexcept -> bool final;
    auto get_contact() const noexcept -> identifier::Generic final
    {
        return contact_id_;
    }
    auto has_private(const PasswordPrompt& reason) const noexcept -> bool;
    auto save(const rLock& lock) const noexcept -> bool final;

    PaymentCodePrivate(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Account& parent,
        const identifier::Account& id,
        const SerializedType& serialized,
        opentxs::PaymentCode local,
        opentxs::PaymentCode remote,
        identifier::Generic contact) noexcept(false);
};
}  // namespace opentxs::blockchain::crypto
