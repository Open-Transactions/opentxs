// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "core/paymentcode/PaymentCode.hpp"
#include "core/paymentcode/Preimage.hpp"
#include "internal/core/PaymentCode.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::implementation
{
class PaymentCode final : public opentxs::PaymentCode::Imp
{
public:
    using ECKey = std::unique_ptr<crypto::key::EllipticCurve>;
    using HDKey = std::shared_ptr<crypto::key::HD>;

    static const std::size_t pubkey_size_;
    static const std::size_t chain_code_size_;

    operator const opentxs::crypto::key::Asymmetric&() const noexcept final;

    auto operator==(const proto::PaymentCode& rhs) const noexcept -> bool final;

    auto asBase58() const noexcept -> UnallocatedCString final;
    auto Blind(
        const opentxs::PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const ReadView outpoint,
        const AllocateOutput destination,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool final;
    auto BlindV3(
        const opentxs::PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const AllocateOutput destination,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool final;
    auto clone() const noexcept -> PaymentCode* final
    {
        return std::make_unique<PaymentCode>(*this).release();
    }
    auto DecodeNotificationElements(
        const std::uint8_t version,
        const UnallocatedVector<Space>& elements,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode final;
    auto GenerateNotificationElements(
        const opentxs::PaymentCode& recipient,
        const crypto::key::EllipticCurve& privateKey,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> UnallocatedVector<Space> final;
    auto ID() const noexcept -> const identifier::Nym& final { return id_; }
    auto Incoming(
        const opentxs::PaymentCode& sender,
        const Bip32Index index,
        const blockchain::Type chain,
        const opentxs::PasswordPrompt& reason,
        const std::uint8_t version) const noexcept
        -> std::unique_ptr<crypto::key::EllipticCurve> final;
    auto Key() const noexcept -> std::shared_ptr<crypto::key::HD> final;
    auto Locator(const AllocateOutput destination, const std::uint8_t version)
        const noexcept -> bool final;
    auto Outgoing(
        const opentxs::PaymentCode& recipient,
        const Bip32Index index,
        const blockchain::Type chain,
        const opentxs::PasswordPrompt& reason,
        const std::uint8_t version) const noexcept
        -> std::unique_ptr<crypto::key::EllipticCurve> final;
    auto Serialize(AllocateOutput destination) const noexcept -> bool final;
    auto Serialize(Serialized& serialized) const noexcept -> bool final;
    auto Sign(
        const identity::credential::Base& credential,
        proto::Signature& sig,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool final;
    auto Sign(
        const opentxs::Data& data,
        opentxs::Data& output,
        const opentxs::PasswordPrompt& reason) const noexcept -> bool final;
    auto Unblind(
        const ReadView blinded,
        const crypto::key::EllipticCurve& publicKey,
        const ReadView outpoint,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode final;
    auto UnblindV3(
        const std::uint8_t version,
        const ReadView blinded,
        const crypto::key::EllipticCurve& publicKey,
        const opentxs::PasswordPrompt& reason) const noexcept
        -> opentxs::PaymentCode final;
    auto Valid() const noexcept -> bool final;
    auto Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept -> bool final;
    auto Version() const noexcept -> VersionNumber final { return version_; }

    auto AddPrivateKeys(
        UnallocatedCString& seed,
        const Bip32Index index,
        const opentxs::PasswordPrompt& reason) noexcept -> bool final;

    PaymentCode(
        const api::Session& api,
        const std::uint8_t version,
        const bool hasBitmessage,
        const ReadView pubkey,
        const ReadView chaincode,
        const std::uint8_t bitmessageVersion,
        const std::uint8_t bitmessageStream,
        std::unique_ptr<crypto::key::Secp256k1> key) noexcept;
    PaymentCode() = delete;
    PaymentCode(const PaymentCode& rhs) noexcept;
    PaymentCode(PaymentCode&&) = delete;
    auto operator=(const PaymentCode&) -> PaymentCode& = delete;
    auto operator=(PaymentCode&&) -> PaymentCode& = delete;

    ~PaymentCode() final = default;

private:
    using VersionType = std::uint8_t;
    using Mask = std::array<std::byte, 64>;

    const api::Session& api_;
    const VersionType version_;
    const bool has_bitmessage_;
    const ByteArray pubkey_;
    const Secret chain_code_;
    const std::uint8_t bitmessage_version_;
    const std::uint8_t bitmessage_stream_;
    const identifier::Nym id_;
    std::shared_ptr<crypto::key::Secp256k1> key_;

    static auto calculate_id(
        const api::Session& api,
        const ReadView pubkey,
        const ReadView chaincode) noexcept -> identifier::Nym;
    static auto effective_version(
        VersionType requested,
        VersionType actual) noexcept(false) -> VersionType;

    auto apply_mask(const Mask& mask, paymentcode::BinaryPreimage& data)
        const noexcept -> void;
    auto apply_mask(const Mask& mask, paymentcode::BinaryPreimage_3& data)
        const noexcept -> void;
    auto base58_preimage() const noexcept -> paymentcode::Base58Preimage;
    auto base58_preimage_v3() const noexcept -> paymentcode::Base58Preimage_3;
    auto binary_preimage() const noexcept -> paymentcode::BinaryPreimage;
    auto binary_preimage_v3() const noexcept -> paymentcode::BinaryPreimage_3;
    auto calculate_mask_v1(
        const crypto::key::EllipticCurve& local,
        const crypto::key::EllipticCurve& remote,
        const ReadView outpoint,
        const opentxs::PasswordPrompt& reason) const noexcept(false) -> Mask;
    auto calculate_mask_v3(
        const crypto::key::EllipticCurve& local,
        const crypto::key::EllipticCurve& remote,
        const ReadView pubkey,
        const opentxs::PasswordPrompt& reason) const noexcept(false) -> Mask;
    auto derive_keys(
        const opentxs::PaymentCode& other,
        const Bip32Index local,
        const Bip32Index remote,
        const opentxs::PasswordPrompt& reason) const noexcept(false)
        -> std::pair<
            std::unique_ptr<crypto::key::EllipticCurve>,
            std::unique_ptr<crypto::key::EllipticCurve>>;
    auto effective_version(VersionType in) const noexcept(false) -> VersionType
    {
        return effective_version(in, version_);
    }
    auto generate_elements_v1(
        const opentxs::PaymentCode& recipient,
        const Space& blind,
        UnallocatedVector<Space>& output) const noexcept(false) -> void;
    auto generate_elements_v3(
        const opentxs::PaymentCode& recipient,
        const Space& blind,
        UnallocatedVector<Space>& output) const noexcept(false) -> void;
    auto match_locator(const std::uint8_t version, const Space& element) const
        noexcept(false) -> bool;
    auto postprocess(const Secret& in) const noexcept(false) -> Secret;
    auto shared_secret_mask_v1(
        const crypto::key::EllipticCurve& local,
        const crypto::key::EllipticCurve& remote,
        const opentxs::PasswordPrompt& reason) const noexcept(false) -> Secret;
    auto shared_secret_payment_v1(
        const crypto::key::EllipticCurve& local,
        const crypto::key::EllipticCurve& remote,
        const opentxs::PasswordPrompt& reason) const noexcept(false) -> Secret;
    auto shared_secret_payment_v3(
        const crypto::key::EllipticCurve& local,
        const crypto::key::EllipticCurve& remote,
        const blockchain::Type chain,
        const opentxs::PasswordPrompt& reason) const noexcept(false) -> Secret;
    auto unblind_v1(
        const ReadView in,
        const Mask& mask,
        const crypto::EcdsaProvider& ecdsa,
        const opentxs::PasswordPrompt& reason) const -> opentxs::PaymentCode;
    auto unblind_v3(
        const std::uint8_t version,
        const ReadView in,
        const Mask& mask,
        const crypto::EcdsaProvider& ecdsa,
        const opentxs::PasswordPrompt& reason) const -> opentxs::PaymentCode;
};
}  // namespace opentxs::implementation
