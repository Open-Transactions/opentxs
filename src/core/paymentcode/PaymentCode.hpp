// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
namespace key
{
class EllipticCurve;
class HD;
class Secp256k1;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class PaymentCode::Imp : virtual public internal::PaymentCode
{
public:
    virtual operator const opentxs::crypto::key::Asymmetric&()
        const noexcept = 0;

    virtual auto asBase58() const noexcept -> UnallocatedCString = 0;
    virtual auto Blind(
        const opentxs::PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const ReadView outpoint,
        const AllocateOutput destination,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto BlindV3(
        const opentxs::PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const AllocateOutput destination,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto clone() const noexcept -> Imp* = 0;
    virtual auto DecodeNotificationElements(
        const std::uint8_t version,
        const UnallocatedVector<Space>& elements,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto GenerateNotificationElements(
        const opentxs::PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const PasswordPrompt& reason) const noexcept
        -> UnallocatedVector<Space> = 0;
    virtual auto ID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Incoming(
        const opentxs::PaymentCode& sender,
        const Bip32Index index,
        const blockchain::Type chain,
        const PasswordPrompt& reason,
        const std::uint8_t version) const noexcept
        -> std::unique_ptr<crypto::key::EllipticCurve> = 0;
    virtual auto Key() const noexcept -> std::shared_ptr<crypto::key::HD> = 0;
    virtual auto Locator(
        const AllocateOutput destination,
        const std::uint8_t version) const noexcept -> bool = 0;
    virtual auto Outgoing(
        const opentxs::PaymentCode& recipient,
        const Bip32Index index,
        const blockchain::Type chain,
        const PasswordPrompt& reason,
        const std::uint8_t version) const noexcept
        -> std::unique_ptr<crypto::key::EllipticCurve> = 0;
    using internal::PaymentCode::Serialize;
    virtual auto Serialize(AllocateOutput destination) const noexcept
        -> bool = 0;
    using internal::PaymentCode::Sign;
    virtual auto Sign(
        const Data& data,
        Data& output,
        const PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto Unblind(
        const ReadView blinded,
        const crypto::key::EllipticCurve& publicKey,
        const ReadView outpoint,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto UnblindV3(
        const std::uint8_t version,
        const ReadView blinded,
        const crypto::key::EllipticCurve& publicKey,
        const PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode = 0;
    virtual auto Valid() const noexcept -> bool = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    ~Imp() override = default;

protected:
    Imp() = default;

private:
    Imp(const Imp& rhs) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp&;
    auto operator=(Imp&&) -> Imp&;
};
}  // namespace opentxs

namespace opentxs::blank
{
class PaymentCode final : public opentxs::PaymentCode::Imp
{
public:
    operator const opentxs::crypto::key::Asymmetric&() const noexcept final
    {
        static const auto blank = crypto::key::blank::Asymmetric{};

        return blank;
    }
    auto operator==(const proto::PaymentCode&) const noexcept -> bool final
    {
        return {};
    }

    auto asBase58() const noexcept -> UnallocatedCString final { return {}; }
    auto Blind(
        const opentxs::PaymentCode&,
        const crypto::key::EllipticCurve&,
        const ReadView,
        const AllocateOutput,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return {};
    }
    auto BlindV3(
        const opentxs::PaymentCode&,
        const crypto::key::EllipticCurve&,
        const AllocateOutput,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return {};
    }
    auto clone() const noexcept -> PaymentCode* final { return {}; }
    auto DecodeNotificationElements(
        const std::uint8_t,
        const UnallocatedVector<Space>&,
        const PasswordPrompt&) const noexcept -> opentxs::PaymentCode final
    {
        return std::make_unique<PaymentCode>().release();
    }
    auto GenerateNotificationElements(
        const opentxs::PaymentCode&,
        const crypto::key::EllipticCurve&,
        const PasswordPrompt&) const noexcept -> UnallocatedVector<Space> final
    {
        return {};
    }
    auto ID() const noexcept -> const identifier::Nym& final
    {
        static const auto blank = identifier::Nym::Factory();

        return blank;
    }
    auto Incoming(
        const opentxs::PaymentCode&,
        const Bip32Index,
        const blockchain::Type,
        const PasswordPrompt&,
        const std::uint8_t) const noexcept
        -> std::unique_ptr<crypto::key::EllipticCurve> final
    {
        return {};
    }
    auto Key() const noexcept -> std::shared_ptr<crypto::key::HD> final
    {
        return {};
    }
    auto Locator(const AllocateOutput, const std::uint8_t) const noexcept
        -> bool final
    {
        return {};
    }
    auto Outgoing(
        const opentxs::PaymentCode&,
        const Bip32Index,
        const blockchain::Type,
        const PasswordPrompt&,
        const std::uint8_t) const noexcept
        -> std::unique_ptr<crypto::key::EllipticCurve> final
    {
        return {};
    }
    using internal::PaymentCode::Serialize;
    auto Serialize(AllocateOutput) const noexcept -> bool final { return {}; }
    auto Serialize(Serialized& serialized) const noexcept -> bool final
    {
        return {};
    }
    auto Sign(
        const identity::credential::Base&,
        proto::Signature&,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return {};
    }
    auto Sign(const Data&, Data&, const PasswordPrompt&) const noexcept
        -> bool final
    {
        return {};
    }
    auto Unblind(
        const ReadView,
        const crypto::key::EllipticCurve&,
        const ReadView,
        const PasswordPrompt&) const noexcept -> opentxs::PaymentCode final
    {
        return std::make_unique<PaymentCode>().release();
    }
    auto UnblindV3(
        const std::uint8_t,
        const ReadView,
        const crypto::key::EllipticCurve&,
        const PasswordPrompt&) const noexcept -> opentxs::PaymentCode final
    {
        return std::make_unique<PaymentCode>().release();
    }
    auto Valid() const noexcept -> bool final { return {}; }
    auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool final
    {
        return {};
    }
    auto Version() const noexcept -> VersionNumber final { return {}; }

    auto AddPrivateKeys(
        UnallocatedCString&,
        const Bip32Index,
        const PasswordPrompt&) noexcept -> bool final
    {
        return {};
    }

    PaymentCode() = default;
    ~PaymentCode() override = default;
};
}  // namespace opentxs::blank
