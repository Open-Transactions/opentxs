// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/paymentcode/PaymentCode.hpp"  // IWYU pragma: associated

#include <compare>
#include <memory>
#include <utility>

#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/key/HD.hpp"             // IWYU pragma: keep
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
PaymentCode::Imp::Imp() noexcept = default;

auto PaymentCode::Imp::AddPrivateKeys(
    const crypto::SeedID&,
    const crypto::Bip32Index,
    const opentxs::PasswordPrompt&) noexcept -> bool
{
    return {};
}

PaymentCode::Imp::operator const opentxs::crypto::asymmetric::Key&()
    const noexcept
{
    return crypto::asymmetric::Key::Blank();
}

auto PaymentCode::Imp::asBase58() const noexcept -> UnallocatedCString
{
    return {};
}

auto PaymentCode::Imp::Blind(
    const opentxs::PaymentCode&,
    const crypto::asymmetric::key::EllipticCurve&,
    const ReadView,
    Writer&&,
    const opentxs::PasswordPrompt&) const noexcept -> bool
{
    return {};
}

auto PaymentCode::Imp::BlindV3(
    const opentxs::PaymentCode&,
    const crypto::asymmetric::key::EllipticCurve&,
    Writer&&,
    const opentxs::PasswordPrompt&) const noexcept -> bool
{
    return {};
}

auto PaymentCode::Imp::clone() const noexcept -> Imp* { return {}; }

auto PaymentCode::Imp::DecodeNotificationElements(
    const std::uint8_t,
    const UnallocatedVector<Space>&,
    const opentxs::PasswordPrompt&) const noexcept -> opentxs::PaymentCode
{
    return std::make_unique<Imp>().release();
}

auto PaymentCode::Imp::GenerateNotificationElements(
    const opentxs::PaymentCode&,
    const crypto::asymmetric::key::EllipticCurve&,
    const opentxs::PasswordPrompt&) const noexcept -> UnallocatedVector<Space>
{
    return {};
}

auto PaymentCode::Imp::ID() const noexcept -> const identifier::Nym&
{
    static const auto blank = identifier::Nym{};

    return blank;
}
auto PaymentCode::Imp::Incoming(
    const opentxs::PaymentCode&,
    const crypto::Bip32Index,
    const blockchain::Type,
    const opentxs::PasswordPrompt&,
    const std::uint8_t) const noexcept -> crypto::asymmetric::key::EllipticCurve
{
    return {};
}

auto PaymentCode::Imp::Key() const noexcept
    -> const crypto::asymmetric::key::HD&
{
    return crypto::asymmetric::key::HD::Blank();
}

auto PaymentCode::Imp::Locator(Writer&&, const std::uint8_t) const noexcept
    -> bool
{
    return {};
}

auto PaymentCode::Imp::operator==(const protobuf::PaymentCode&) const noexcept
    -> bool
{
    return {};
}

auto PaymentCode::Imp::Outgoing(
    const opentxs::PaymentCode&,
    const crypto::Bip32Index,
    const blockchain::Type,
    const opentxs::PasswordPrompt&,
    const std::uint8_t) const noexcept -> crypto::asymmetric::key::EllipticCurve
{
    return {};
}

auto PaymentCode::Imp::Serialize(Writer&&) const noexcept -> bool { return {}; }

auto PaymentCode::Imp::Serialize(Serialized& serialized) const noexcept -> bool
{
    return {};
}

auto PaymentCode::Imp::Sign(
    const identity::credential::Base&,
    protobuf::Signature&,
    const opentxs::PasswordPrompt&) const noexcept -> bool
{
    return {};
}

auto PaymentCode::Imp::Sign(const Data&, Data&, const opentxs::PasswordPrompt&)
    const noexcept -> bool
{
    return {};
}

auto PaymentCode::Imp::Unblind(
    const ReadView,
    const crypto::asymmetric::key::EllipticCurve&,
    const ReadView,
    const opentxs::PasswordPrompt&) const noexcept -> opentxs::PaymentCode
{
    return std::make_unique<Imp>().release();
}

auto PaymentCode::Imp::UnblindV3(
    const std::uint8_t,
    const ReadView,
    const crypto::asymmetric::key::EllipticCurve&,
    const opentxs::PasswordPrompt&) const noexcept -> opentxs::PaymentCode
{
    return std::make_unique<Imp>().release();
}

auto PaymentCode::Imp::Valid() const noexcept -> bool { return {}; }

auto PaymentCode::Imp::Verify(
    const protobuf::Credential& master,
    const protobuf::Signature& sourceSignature) const noexcept -> bool
{
    return {};
}

auto PaymentCode::Imp::Version() const noexcept -> VersionNumber { return {}; }

PaymentCode::Imp::~Imp() = default;
}  // namespace opentxs

namespace opentxs
{
auto swap(PaymentCode& lhs, PaymentCode& rhs) noexcept -> void
{
    lhs.swap(rhs);
}

auto operator==(const PaymentCode& lhs, const PaymentCode& rhs) noexcept -> bool
{
    return lhs.ID() == rhs.ID();
}

auto operator<=>(const PaymentCode& lhs, const PaymentCode& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.ID() <=> rhs.ID();
}

PaymentCode::PaymentCode(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

PaymentCode::PaymentCode() noexcept
    : PaymentCode(std::make_unique<Imp>().release())
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

    assert_false(nullptr == temp);

    imp_ = rhs.imp_->clone();

    return *this;
}

auto PaymentCode::operator=(PaymentCode&& rhs) noexcept -> PaymentCode&
{
    swap(rhs);

    return *this;
}

PaymentCode::operator const crypto::asymmetric::Key&() const noexcept
{
    return *imp_;
}

auto PaymentCode::asBase58() const noexcept -> UnallocatedCString
{
    return imp_->asBase58();
}

auto PaymentCode::Blind(
    const PaymentCode& recipient,
    const crypto::asymmetric::key::EllipticCurve& privateKey,
    const ReadView outpoint,
    Writer&& destination,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->Blind(
        recipient, privateKey, outpoint, std::move(destination), reason);
}

auto PaymentCode::BlindV3(
    const PaymentCode& recipient,
    const crypto::asymmetric::key::EllipticCurve& privateKey,
    Writer&& destination,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->BlindV3(recipient, privateKey, std::move(destination), reason);
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
    const crypto::asymmetric::key::EllipticCurve& privateKey,
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

auto PaymentCode::Incoming(
    const PaymentCode& sender,
    const crypto::Bip32Index index,
    const blockchain::Type chain,
    const opentxs::PasswordPrompt& reason,
    const std::uint8_t version) const noexcept
    -> crypto::asymmetric::key::EllipticCurve
{
    return imp_->Incoming(sender, index, chain, reason, version);
}

auto PaymentCode::Key() const noexcept -> const crypto::asymmetric::key::HD&
{
    return imp_->Key();
}

auto PaymentCode::Locator(Writer&& destination, const std::uint8_t version)
    const noexcept -> bool
{
    return imp_->Locator(std::move(destination), version);
}

auto PaymentCode::Outgoing(
    const PaymentCode& recipient,
    const crypto::Bip32Index index,
    const blockchain::Type chain,
    const opentxs::PasswordPrompt& reason,
    const std::uint8_t version) const noexcept
    -> crypto::asymmetric::key::EllipticCurve
{
    return imp_->Outgoing(recipient, index, chain, reason, version);
}

auto PaymentCode::Serialize(Writer&& destination) const noexcept -> bool
{
    return imp_->Serialize(std::move(destination));
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
    const crypto::asymmetric::key::EllipticCurve& publicKey,
    const ReadView outpoint,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> opentxs::PaymentCode
{
    return imp_->Unblind(blinded, publicKey, outpoint, reason);
}

auto PaymentCode::UnblindV3(
    const std::uint8_t version,
    const ReadView blinded,
    const crypto::asymmetric::key::EllipticCurve& publicKey,
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
