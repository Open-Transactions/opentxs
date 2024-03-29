// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstdint>
#include <functional>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key

class Key;
}  // namespace asymmetric
}  // namespace crypto

namespace identifier
{
class Nym;
}  // namespace identifier

namespace internal
{
class PaymentCode;
}  // namespace internal

class Data;
class PaymentCode;
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct hash<opentxs::PaymentCode> {
    auto operator()(const opentxs::PaymentCode& rhs) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs
{
class OPENTXS_EXPORT PaymentCode
{
public:
    class Imp;

    static auto DefaultVersion() noexcept -> VersionNumber;

    operator bool() const noexcept { return Valid(); }
    operator const crypto::asymmetric::Key&() const noexcept;

    auto asBase58() const noexcept -> UnallocatedCString;
    auto Blind(
        const PaymentCode& recipient,
        const crypto::asymmetric::key::EllipticCurve& privateKey,
        const ReadView outpoint,
        Writer&& destination,
        const PasswordPrompt& reason) const noexcept -> bool;
    auto BlindV3(
        const PaymentCode& recipient,
        const crypto::asymmetric::key::EllipticCurve& privateKey,
        Writer&& destination,
        const PasswordPrompt& reason) const noexcept -> bool;
    auto DecodeNotificationElements(
        const std::uint8_t version,
        const UnallocatedVector<Space>& elements,
        const PasswordPrompt& reason) const noexcept -> opentxs::PaymentCode;
    auto GenerateNotificationElements(
        const PaymentCode& recipient,
        const crypto::asymmetric::key::EllipticCurve& privateKey,
        const PasswordPrompt& reason) const noexcept
        -> UnallocatedVector<Space>;
    auto ID() const noexcept -> const identifier::Nym&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::PaymentCode&;
    auto Incoming(
        const PaymentCode& sender,
        const crypto::Bip32Index index,
        const blockchain::Type chain,
        const PasswordPrompt& reason,
        const std::uint8_t version = 0) const noexcept
        -> crypto::asymmetric::key::EllipticCurve;
    auto Key() const noexcept -> const crypto::asymmetric::key::HD&;
    auto Locator(Writer&& destination, const std::uint8_t version = 0)
        const noexcept -> bool;
    auto Outgoing(
        const PaymentCode& recipient,
        const crypto::Bip32Index index,
        const blockchain::Type chain,
        const PasswordPrompt& reason,
        const std::uint8_t version = 0) const noexcept
        -> crypto::asymmetric::key::EllipticCurve;
    auto Serialize(Writer&& destination) const noexcept -> bool;
    auto Sign(const Data& data, Data& output, const PasswordPrompt& reason)
        const noexcept -> bool;
    auto Unblind(
        const ReadView blinded,
        const crypto::asymmetric::key::EllipticCurve& publicKey,
        const ReadView outpoint,
        const PasswordPrompt& reason) const noexcept -> opentxs::PaymentCode;
    auto UnblindV3(
        const std::uint8_t version,
        const ReadView blinded,
        const crypto::asymmetric::key::EllipticCurve& publicKey,
        const PasswordPrompt& reason) const noexcept -> opentxs::PaymentCode;
    auto Valid() const noexcept -> bool;
    auto Version() const noexcept -> VersionNumber;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::PaymentCode&;
    auto swap(PaymentCode& rhs) noexcept -> void;

    PaymentCode() noexcept;
    OPENTXS_NO_EXPORT PaymentCode(Imp*) noexcept;
    PaymentCode(const PaymentCode&) noexcept;
    PaymentCode(PaymentCode&&) noexcept;
    auto operator=(const PaymentCode&) noexcept -> PaymentCode&;
    auto operator=(PaymentCode&&) noexcept -> PaymentCode&;

    virtual ~PaymentCode();

private:
    Imp* imp_;
};

OPENTXS_EXPORT auto swap(PaymentCode& lhs, PaymentCode& rhs) noexcept -> void;
OPENTXS_EXPORT auto operator==(
    const PaymentCode& lhs,
    const PaymentCode& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(
    const PaymentCode& lhs,
    const PaymentCode& rhs) noexcept -> std::strong_ordering;
}  // namespace opentxs
