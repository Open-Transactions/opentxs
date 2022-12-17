// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/asymmetric/Key.hpp"

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/core/PaymentCode.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
class HD;
class Secp256k1;
}  // namespace key

class Key;
}  // namespace asymmetric

class EcdsaProvider;
}  // namespace crypto

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace credential
{
class Base;
}  // namespace credential
}  // namespace identity

namespace proto
{
class Credential;
class PaymentCode;
class Signature;
}  // namespace proto

class Data;
class PasswordPrompt;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class PaymentCode::Imp : virtual public internal::PaymentCode
{
public:
    virtual operator const opentxs::crypto::asymmetric::Key&() const noexcept;

    virtual auto asBase58() const noexcept -> UnallocatedCString;
    virtual auto Blind(
        const opentxs::PaymentCode& recipient,
        const crypto::asymmetric::key::EllipticCurve& privateKey,
        const ReadView outpoint,
        Writer&& destination,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool;
    virtual auto BlindV3(
        const opentxs::PaymentCode& recipient,
        const crypto::asymmetric::key::EllipticCurve& privateKey,
        Writer&& destination,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool;
    virtual auto clone() const noexcept -> Imp*;
    virtual auto DecodeNotificationElements(
        const std::uint8_t version,
        const UnallocatedVector<Space>& elements,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode;
    virtual auto GenerateNotificationElements(
        const opentxs::PaymentCode& recipient,
        const crypto::asymmetric::key::EllipticCurve& privateKey,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> UnallocatedVector<Space>;
    virtual auto ID() const noexcept -> const identifier::Nym&;
    virtual auto Incoming(
        const opentxs::PaymentCode& sender,
        const Bip32Index index,
        const blockchain::Type chain,
        const opentxs::PasswordPrompt& reason,
        const std::uint8_t version) const noexcept
        -> crypto::asymmetric::key::EllipticCurve;
    virtual auto Key() const noexcept -> const crypto::asymmetric::key::HD&;
    virtual auto Locator(Writer&& destination, const std::uint8_t version)
        const noexcept -> bool;
    auto operator==(const proto::PaymentCode&) const noexcept -> bool override;
    virtual auto Outgoing(
        const opentxs::PaymentCode& recipient,
        const Bip32Index index,
        const blockchain::Type chain,
        const opentxs::PasswordPrompt& reason,
        const std::uint8_t version) const noexcept
        -> crypto::asymmetric::key::EllipticCurve;
    virtual auto Serialize(Writer&& destination) const noexcept -> bool;
    auto Serialize(Serialized& serialized) const noexcept -> bool override;
    auto Sign(
        const identity::credential::Base&,
        proto::Signature&,
        const opentxs::PasswordPrompt&) const noexcept -> bool override;
    virtual auto Sign(
        const Data& data,
        Data& output,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool;
    virtual auto Unblind(
        const ReadView blinded,
        const crypto::asymmetric::key::EllipticCurve& publicKey,
        const ReadView outpoint,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode;
    virtual auto UnblindV3(
        const std::uint8_t version,
        const ReadView blinded,
        const crypto::asymmetric::key::EllipticCurve& publicKey,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode;
    virtual auto Valid() const noexcept -> bool;
    auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept
        -> bool override;
    virtual auto Version() const noexcept -> VersionNumber;

    auto AddPrivateKeys(
        UnallocatedCString&,
        const Bip32Index,
        const opentxs::PasswordPrompt&) noexcept -> bool override;

    Imp() noexcept;
    Imp(const Imp& rhs) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() override;
};
}  // namespace opentxs
