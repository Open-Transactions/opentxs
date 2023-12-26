// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::KeyRole

#pragma once

#include <Enums.pb.h>
#include <frozen/unordered_map.h>
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

namespace proto
{
class AsymmetricKey;
class Credential;
class NymIDSource;
class Signature;
}  // namespace proto

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
    auto Serialize(proto::NymIDSource& serialized) const noexcept -> bool final;
    auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool final;
    auto Sign(
        const identity::credential::Primary& credential,
        proto::Signature& sig,
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
        proto::SourceType,
        SourceTypeMapSize>;
    using SourceTypeReverseMap = frozen::unordered_map<
        proto::SourceType,
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
        const proto::NymIDSource& serialized) -> crypto::asymmetric::Key;
    static auto deserialize_paymentcode(
        const api::session::Factory& factory,
        const identity::SourceType type,
        const proto::NymIDSource& serialized) -> PaymentCode;
    static auto extract_key(
        const proto::Credential& credential,
        const proto::KeyRole role) -> std::unique_ptr<proto::AsymmetricKey>;
    static auto sourcetype_map() noexcept -> const SourceTypeMap&;
    static auto translate(const identity::SourceType in) noexcept
        -> proto::SourceType;
    static auto translate(const proto::SourceType in) noexcept
        -> identity::SourceType;

    auto asData() const -> ByteArray;

    Source(
        const api::Crypto& crypto,
        const api::session::Factory& factory,
        const proto::NymIDSource& serializedSource) noexcept;
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
