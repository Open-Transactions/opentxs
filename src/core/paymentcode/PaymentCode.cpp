// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "core/paymentcode/PaymentCode.hpp"  // IWYU pragma: associated

#include <compare>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/key/HD.hpp"             // IWYU pragma: keep
#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto swap(PaymentCode& lhs, PaymentCode& rhs) noexcept -> void
{
    lhs.swap(rhs);
}

auto operator<(const PaymentCode& lhs, const PaymentCode& rhs) noexcept -> bool
{
    return lhs.ID() < rhs.ID();
}

auto operator==(const PaymentCode& lhs, const PaymentCode& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

PaymentCode::PaymentCode(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

PaymentCode::PaymentCode() noexcept
    : PaymentCode(std::make_unique<blank::PaymentCode>().release())
{
}

PaymentCode::PaymentCode(const PaymentCode& rhs) noexcept
    : PaymentCode(rhs.imp_->clone())
{
}

PaymentCode::PaymentCode(PaymentCode&& rhs) noexcept
    : PaymentCode()
{
    swap(rhs);
}

auto PaymentCode::operator=(const PaymentCode& rhs) noexcept -> PaymentCode&
{
    auto temp = std::unique_ptr<Imp>(imp_);

    OT_ASSERT(temp);

    imp_ = rhs.imp_->clone();

    return *this;
}

auto PaymentCode::operator=(PaymentCode&& rhs) noexcept -> PaymentCode&
{
    swap(rhs);

    return *this;
}

PaymentCode::operator const crypto::key::Asymmetric&() const noexcept
{
    return *imp_;
}

auto PaymentCode::asBase58() const noexcept -> UnallocatedCString
{
    return imp_->asBase58();
}

auto PaymentCode::Blind(
    const PaymentCode& recipient,
    const crypto::key::EllipticCurve& privateKey,
    const ReadView outpoint,
    const AllocateOutput destination,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->Blind(recipient, privateKey, outpoint, destination, reason);
}

auto PaymentCode::BlindV3(
    const PaymentCode& recipient,
    const crypto::key::EllipticCurve& privateKey,
    const AllocateOutput destination,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->BlindV3(recipient, privateKey, destination, reason);
}

auto PaymentCode::DecodeNotificationElements(
    const std::uint8_t version,
    const UnallocatedVector<Space>& elements,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> opentxs::PaymentCode
{
    return imp_->DecodeNotificationElements(version, elements, reason);
}

auto PaymentCode::DefaultVersion() noexcept -> VersionNumber { return 3; }

auto PaymentCode::GenerateNotificationElements(
    const PaymentCode& recipient,
    const crypto::key::EllipticCurve& privateKey,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> UnallocatedVector<Space>
{
    return imp_->GenerateNotificationElements(recipient, privateKey, reason);
}

auto PaymentCode::ID() const noexcept -> const identifier::Nym&
{
    return imp_->ID();
}

auto PaymentCode::Internal() const noexcept -> const internal::PaymentCode&
{
    return *imp_;
}

auto PaymentCode::Internal() noexcept -> internal::PaymentCode&
{
    return *imp_;
}

auto PaymentCode::Key() const noexcept -> std::shared_ptr<crypto::key::HD>
{
    return imp_->Key();
}

auto PaymentCode::Incoming(
    const PaymentCode& sender,
    const Bip32Index index,
    const blockchain::Type chain,
    const opentxs::PasswordPrompt& reason,
    const std::uint8_t version) const noexcept
    -> std::unique_ptr<crypto::key::EllipticCurve>
{
    return imp_->Incoming(sender, index, chain, reason, version);
}

auto PaymentCode::Locator(
    const AllocateOutput destination,
    const std::uint8_t version) const noexcept -> bool
{
    return imp_->Locator(destination, version);
}

auto PaymentCode::Outgoing(
    const PaymentCode& recipient,
    const Bip32Index index,
    const blockchain::Type chain,
    const opentxs::PasswordPrompt& reason,
    const std::uint8_t version) const noexcept
    -> std::unique_ptr<crypto::key::EllipticCurve>
{
    return imp_->Outgoing(recipient, index, chain, reason, version);
}

auto PaymentCode::Serialize(AllocateOutput destination) const noexcept -> bool
{
    return imp_->Serialize(destination);
}

auto PaymentCode::Sign(
    const Data& data,
    Data& output,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->Sign(data, output, reason);
}

auto PaymentCode::swap(PaymentCode& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

auto PaymentCode::Unblind(
    const ReadView blinded,
    const crypto::key::EllipticCurve& publicKey,
    const ReadView outpoint,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> opentxs::PaymentCode
{
    return imp_->Unblind(blinded, publicKey, outpoint, reason);
}

auto PaymentCode::UnblindV3(
    const std::uint8_t version,
    const ReadView blinded,
    const crypto::key::EllipticCurve& publicKey,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> opentxs::PaymentCode
{
    return imp_->UnblindV3(version, blinded, publicKey, reason);
}

auto PaymentCode::Valid() const noexcept -> bool { return imp_->Valid(); }

auto PaymentCode::Version() const noexcept -> VersionNumber
{
    return imp_->Version();
}

PaymentCode::~PaymentCode()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs
