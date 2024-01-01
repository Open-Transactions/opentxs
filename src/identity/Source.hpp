// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::KeyRole

#pragma once

#include <frozen/unordered_map.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <cstddef>
#include <memory>

#include "internal/core/String.hpp"
#include "internal/identity/Source.hpp"
#include "opentxs/Types.internal.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

namespace identity
{
namespace credential
{
class Primary;
}  // namespace credential
}  // namespace identity

namespace protobuf
{
class AsymmetricKey;
class Credential;
class NymIDSource;
class Signature;
}  // namespace protobuf

class ByteArray;
class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::implementation
{
class Source final : public internal::Source
{
public:
    auto asString() const noexcept -> OTString final;
    auto Description() const noexcept -> OTString final;
    auto Type() const noexcept -> identity::SourceType final { return type_; }
    auto NymID() const noexcept -> identifier::Nym final;
    auto Serialize(protobuf::NymIDSource& serialized) const noexcept
        -> bool final;
    auto Verify(
        const protobuf::Credential& master,
        const protobuf::Signature& sourceSignature) const noexcept
        -> bool final;
    auto Sign(
        const identity::credential::Primary& credential,
        protobuf::Signature& sig,
        const PasswordPrompt& reason) const noexcept -> bool final;

    Source() = delete;
    Source(Source&&) = delete;
    auto operator=(const Source&) -> Source& = delete;
    auto operator=(Source&&) -> Source& = delete;

private:
    friend opentxs::Factory;

    static constexpr auto SourceTypeMapSize = std::size_t{3};
    using SourceTypeMap = frozen::unordered_map<
        identity::SourceType,
        protobuf::SourceType,
        SourceTypeMapSize>;
    using SourceTypeReverseMap = frozen::unordered_map<
        protobuf::SourceType,
        identity::SourceType,
        SourceTypeMapSize>;

    static const VersionConversionMap key_to_source_version_;

    const api::Crypto& crypto_;
    const api::session::Factory& factory_;

    identity::SourceType type_;
    crypto::asymmetric::Key pubkey_;
    PaymentCode payment_code_;
    VersionNumber version_;

    static auto deserialize_pubkey(
        const api::session::Factory& factory,
        const identity::SourceType type,
        const protobuf::NymIDSource& serialized) -> crypto::asymmetric::Key;
    static auto deserialize_paymentcode(
        const api::session::Factory& factory,
        const identity::SourceType type,
        const protobuf::NymIDSource& serialized) -> PaymentCode;
    static auto extract_key(
        const protobuf::Credential& credential,
        const protobuf::KeyRole role)
        -> std::unique_ptr<protobuf::AsymmetricKey>;
    static auto sourcetype_map() noexcept -> const SourceTypeMap&;
    static auto translate(const identity::SourceType in) noexcept
        -> protobuf::SourceType;
    static auto translate(const protobuf::SourceType in) noexcept
        -> identity::SourceType;

    auto asData() const -> ByteArray;

    Source(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const protobuf::NymIDSource& serializedSource) noexcept;
    Source(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const crypto::Parameters& nymParameters) noexcept(false);
    Source(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const PaymentCode& source) noexcept;
    Source(const Source& rhs) noexcept;
};
}  // namespace opentxs::identity::implementation
