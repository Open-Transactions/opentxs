// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/paymentcode/Imp.hpp"  // IWYU pragma: associated

#include <Credential.pb.h>
#include <Enums.pb.h>
#include <HDPath.pb.h>
#include <MasterCredentialParameters.pb.h>
#include <NymIDSource.pb.h>
#include <PaymentCode.pb.h>
#include <Signature.pb.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "core/paymentcode/Preimage.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/crypto/asymmetric/Factory.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/asymmetric/key/EllipticCurve.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/identity/Types.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/Credential.hpp"
#include "internal/serialization/protobuf/verify/PaymentCode.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/crypto/SecretStyle.hpp"    // IWYU pragma: keep
#include "opentxs/crypto/SignatureRole.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace be = boost::endian;

namespace opentxs::implementation
{
const std::size_t PaymentCode::pubkey_size_{
    sizeof(paymentcode::XpubPreimage::key_)};
const std::size_t PaymentCode::chain_code_size_{
    sizeof(paymentcode::XpubPreimage::code_)};

PaymentCode::PaymentCode(
    const api::Session& api,
    const std::uint8_t version,
    const bool hasBitmessage,
    const ReadView pubkey,
    const ReadView chaincode,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream,
    crypto::asymmetric::key::Secp256k1 key) noexcept
    : api_(api)
    , version_(version)
    , has_bitmessage_(hasBitmessage)
    , pubkey_(api.Factory().DataFromBytes(pubkey))
    , chain_code_(api.Factory().SecretFromBytes(chaincode))
    , bitmessage_version_(bitmessageVersion)
    , bitmessage_stream_(bitmessageStream)
    , id_(calculate_id(api, pubkey, chaincode))
    , key_(std::move(key))
{
    OT_ASSERT(id_.Type() == identifier::Type::nym);
}

PaymentCode::PaymentCode(const PaymentCode& rhs) noexcept
    : api_(rhs.api_)
    , version_(rhs.version_)
    , has_bitmessage_(rhs.has_bitmessage_)
    , pubkey_(rhs.pubkey_)
    , chain_code_(rhs.chain_code_)
    , bitmessage_version_(rhs.bitmessage_version_)
    , bitmessage_stream_(rhs.bitmessage_stream_)
    , id_(rhs.id_)
    , key_(rhs.key_)
{
    OT_ASSERT(id_.Type() == identifier::Type::nym);
}

PaymentCode::operator const crypto::asymmetric::Key&() const noexcept
{
    return key_;
}

auto PaymentCode::operator==(const proto::PaymentCode& rhs) const noexcept
    -> bool
{
    auto lhs = proto::PaymentCode{};

    if (false == Serialize(lhs)) { return false; }

    const auto LHData = api_.Factory().Internal().Data(lhs);
    const auto RHData = api_.Factory().Internal().Data(rhs);

    return (LHData == RHData);
}

auto PaymentCode::AddPrivateKeys(
    const crypto::SeedID& seed,
    const Bip32Index index,
    const opentxs::PasswordPrompt& reason) noexcept -> bool
{
    auto candidate =
        api_.Crypto().Seed().GetPaymentCode(seed, index, version_, reason);

    if (false == candidate.IsValid()) {
        LogError()(OT_PRETTY_CLASS())("Failed to derive private key").Flush();

        return false;
    }

    if (0 != pubkey_.Bytes().compare(candidate.PublicKey())) {
        LogError()(OT_PRETTY_CLASS())(
            "Derived public key does not match this payment code")
            .Flush();

        return false;
    }

    if (0 != chain_code_.Bytes().compare(candidate.Chaincode(reason))) {
        LogError()(OT_PRETTY_CLASS())(
            "Derived chain code does not match this payment code")
            .Flush();

        return false;
    }

    key_ = std::move(candidate);

    return key_.IsValid();
}

auto PaymentCode::apply_mask(const Mask& mask, paymentcode::BinaryPreimage& pre)
    const noexcept -> void
{
    static_assert(80 == sizeof(pre));

    auto* i = reinterpret_cast<std::byte*>(&pre);
    std::advance(i, 3);

    for (const auto& m : mask) {
        auto& byte = *i;
        byte ^= m;
        std::advance(i, 1);
    }
}

auto PaymentCode::apply_mask(
    const Mask& mask,
    paymentcode::BinaryPreimage_3& pre) const noexcept -> void
{
    static_assert(34 == sizeof(pre));
    // NOLINTBEGIN(readability-qualified-auto)
    auto i = std::next(pre.key_.begin());
    auto const end = pre.key_.end();
    // NOLINTEND(readability-qualified-auto)

    for (const auto& m : mask) {
        auto& byte = *i;
        byte ^= m;
        std::advance(i, 1);

        if (i == end) { break; }
    }
}

auto PaymentCode::asBase58() const noexcept -> UnallocatedCString
{
    try {
        auto out = UnallocatedCString{};
        const auto preimage = [&]() -> ByteArray {
            switch (version_) {
                case 1:
                case 2: {
                    return static_cast<ReadView>(base58_preimage());
                }
                case 3:
                default: {
                    return static_cast<ReadView>(base58_preimage_v3());
                }
            }
        }();
        const auto rc = api_.Crypto().Encode().Base58CheckEncode(
            preimage.Bytes(), writer(out));

        if (false == rc) { throw std::runtime_error{"base58 encode error"}; }

        return out;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto PaymentCode::base58_preimage() const noexcept
    -> paymentcode::Base58Preimage
{
    return paymentcode::Base58Preimage{binary_preimage()};
}

auto PaymentCode::base58_preimage_v3() const noexcept
    -> paymentcode::Base58Preimage_3
{
    return paymentcode::Base58Preimage_3{binary_preimage_v3()};
}

auto PaymentCode::binary_preimage() const noexcept
    -> paymentcode::BinaryPreimage
{
    return paymentcode::BinaryPreimage{
        version_,
        has_bitmessage_,
        pubkey_.Bytes(),
        chain_code_.Bytes(),
        bitmessage_version_,
        bitmessage_stream_};
}

auto PaymentCode::binary_preimage_v3() const noexcept
    -> paymentcode::BinaryPreimage_3
{
    return paymentcode::BinaryPreimage_3{version_, pubkey_.Bytes()};
}

auto PaymentCode::Blind(
    const opentxs::PaymentCode& recipient,
    const crypto::asymmetric::key::EllipticCurve& privateKey,
    const ReadView outpoint,
    Writer&& dest,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    try {
        if (false == recipient.Key().IsValid()) {
            throw std::runtime_error{"remote payment code key is invalid"};
        }

        if (id_ == recipient.ID()) {
            throw std::runtime_error{"remote payment code must be different "
                                     "than local payment code"};
        }

        if (2 < recipient.Version()) {
            throw std::runtime_error{"Recipient payment code version too high"};
        }

        const auto& hd = recipient.Key();

        if (false == hd.IsValid()) {
            throw std::runtime_error{"Failed to obtain remote hd key"};
        }

        const auto remotePublic = hd.ChildKey(0, reason);

        if (false == remotePublic.IsValid()) {
            throw std::runtime_error{
                "Failed to derive remote notification key"};
        }

        const auto mask =
            calculate_mask_v1(privateKey, remotePublic, outpoint, reason);
        auto pre = binary_preimage();
        apply_mask(mask, pre);

        return copy(pre, std::move(dest));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto PaymentCode::BlindV3(
    const opentxs::PaymentCode& recipient,
    const crypto::asymmetric::key::EllipticCurve& privateKey,
    Writer&& dest,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    try {
        if (false == recipient.Key().IsValid()) {
            throw std::runtime_error{"remote payment code key is invalid"};
        }

        if (id_ == recipient.ID()) {
            throw std::runtime_error{"remote payment code must be different "
                                     "than local payment code"};
        }

        if (3 > recipient.Version()) {
            throw std::runtime_error{"Recipient payment code version too low"};
        }

        const auto& hd = recipient.Key();

        if (false == hd.IsValid()) {
            throw std::runtime_error{"Failed to obtain remote hd key"};
        }

        const auto remotePublic = hd.ChildKey(0, reason);

        if (false == remotePublic.IsValid()) {
            throw std::runtime_error{
                "Failed to derive remote notification key"};
        }

        const auto mask = calculate_mask_v3(
            privateKey, remotePublic, privateKey.PublicKey(), reason);

        switch (version_) {
            case 1:
            case 2: {
                auto pre = binary_preimage();
                apply_mask(mask, pre);
                copy(pre, std::move(dest));
            } break;
            case 3:
            default: {
                auto pre = binary_preimage_v3();
                apply_mask(mask, pre);
                copy(reader(pre.key_), std::move(dest));
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }

    return true;
}

auto PaymentCode::calculate_id(
    const api::Session& api,
    const ReadView key,
    const ReadView code) noexcept -> identifier::Nym
{
    if ((nullptr == key.data()) || (nullptr == code.data())) { return {}; }

    auto preimage = api.Factory().Data();
    const auto target{pubkey_size_ + chain_code_size_};
    auto raw = preimage.WriteInto().Reserve(target);

    OT_ASSERT(raw.IsValid(target));

    auto* it = raw.as<std::byte>();
    std::memcpy(
        it, key.data(), std::min(key.size(), std::size_t{pubkey_size_}));
    std::advance(it, pubkey_size_);
    std::memcpy(
        it, code.data(), std::min(code.size(), std::size_t{chain_code_size_}));

    return api.Factory().NymIDFromPreimage(preimage.Bytes());
}

auto PaymentCode::calculate_mask_v1(
    const crypto::asymmetric::key::EllipticCurve& local,
    const crypto::asymmetric::key::EllipticCurve& remote,
    const ReadView outpoint,
    const opentxs::PasswordPrompt& reason) const noexcept(false) -> Mask
{
    auto mask = Mask{};
    const auto secret = shared_secret_mask_v1(local, remote, reason);
    const auto hashed = api_.Crypto().Hash().HMAC(
        crypto::HashType::Sha512,
        outpoint,
        secret.Bytes(),
        preallocated(mask.size(), mask.data()));

    if (false == hashed) { throw std::runtime_error{"Failed to derive mask"}; }

    return mask;
}

auto PaymentCode::calculate_mask_v3(
    const crypto::asymmetric::key::EllipticCurve& local,
    const crypto::asymmetric::key::EllipticCurve& remote,
    const ReadView pubkey,
    const opentxs::PasswordPrompt& reason) const noexcept(false) -> Mask
{
    auto mask = Mask{};
    const auto secret = shared_secret_mask_v1(local, remote, reason);
    const auto hashed = api_.Crypto().Hash().HMAC(
        crypto::HashType::Sha512,
        secret.Bytes(),
        pubkey,
        preallocated(mask.size(), mask.data()));

    if (false == hashed) { throw std::runtime_error{"Failed to derive mask"}; }

    return mask;
}

auto PaymentCode::DecodeNotificationElements(
    const std::uint8_t version,
    const UnallocatedVector<Space>& in,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> opentxs::PaymentCode
{
    try {
        if (3 > version_) {
            throw std::runtime_error{"Payment code version too low"};
        }

        if (3 != in.size()) {
            throw std::runtime_error{"Wrong number of elements"};
        }

        const auto& A = in.at(0);
        const auto& F = in.at(1);
        const auto& G = in.at(2);

        if (33 != A.size()) {
            throw std::runtime_error{
                UnallocatedCString{"Invalid A ("} + std::to_string(A.size()) +
                ") bytes"};
        }

        if (false == match_locator(version, F)) {
            throw std::runtime_error{"Invalid locator"};
        }

        const auto blind = [&] {
            if (2 < version) {
                if (33 != G.size()) {
                    throw std::runtime_error{
                        UnallocatedCString{"Invalid G ("} +
                        std::to_string(G.size()) + ") bytes"};
                }

                return G;
            } else {
                if (65 != F.size()) {
                    throw std::runtime_error{
                        UnallocatedCString{"Invalid F ("} +
                        std::to_string(F.size()) + ") bytes"};
                }
                if (65 != G.size()) {
                    throw std::runtime_error{
                        UnallocatedCString{"Invalid G ("} +
                        std::to_string(G.size()) + ") bytes"};
                }

                auto out = space(sizeof(paymentcode::BinaryPreimage));
                auto* o = out.data();
                {
                    const auto* f = F.data();
                    std::advance(f, 33);
                    std::memcpy(o, f, 32);
                    std::advance(o, 32);
                }
                {
                    const auto* g = G.data();
                    std::advance(g, 1);
                    std::memcpy(o, g, 48);
                }

                return out;
            }
        }();

        const auto key = api_.Crypto().Asymmetric().InstantiateSecp256k1Key(
            reader(A), reason);

        if (false == key.IsValid()) {
            throw std::runtime_error{"Failed to instantiate public key"};
        }

        return UnblindV3(version, reader(blind), key, reason);
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto PaymentCode::derive_keys(
    const opentxs::PaymentCode& other,
    const Bip32Index local,
    const Bip32Index remote,
    const opentxs::PasswordPrompt& reason) const noexcept(false)
    -> std::pair<
        crypto::asymmetric::key::EllipticCurve,
        crypto::asymmetric::key::EllipticCurve>
{
    auto output = std::pair<
        crypto::asymmetric::key::EllipticCurve,
        crypto::asymmetric::key::EllipticCurve>{};
    auto& [localPrivate, remotePublic] = output;

    if (key_.IsValid()) {
        localPrivate = key_.ChildKey(local, reason);

        if (false == localPrivate.IsValid()) {
            throw std::runtime_error("Failed to derive local private key");
        }
    } else {
        throw std::runtime_error("Failed to obtain local hd key");
    }

    if (const auto& key = other.Key(); key.IsValid()) {
        remotePublic = key.ChildKey(remote, reason);

        if (false == remotePublic.IsValid()) {
            throw std::runtime_error("Failed to derive remote public key");
        }
    } else {
        throw std::runtime_error("Failed to obtain remote hd key");
    }

    return output;
}

auto PaymentCode::effective_version(
    VersionType requested,
    VersionType actual) noexcept(false) -> VersionType
{
    auto version = (0 == requested) ? actual : requested;

    if (version > actual) {
        const auto error = UnallocatedCString{"Requested version ("} +
                           std::to_string(version) +
                           ") is higher than allowed (" +
                           std::to_string(actual) + ")";

        throw std::runtime_error(error);
    }

    return version;
}

auto PaymentCode::GenerateNotificationElements(
    const opentxs::PaymentCode& recipient,
    const crypto::asymmetric::key::EllipticCurve& privateKey,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> UnallocatedVector<Space>
{
    try {
        if (false == recipient.Key().IsValid()) {
            throw std::runtime_error{"remote payment code key is invalid"};
        }

        if (id_ == recipient.ID()) {
            throw std::runtime_error{"remote payment code must be different "
                                     "than local payment code"};
        }

        if (3 > recipient.Version()) {
            throw std::runtime_error{"Recipient payment code version too low"};
        }

        const auto blind = [&] {
            auto out = Space{};
            const auto rc = BlindV3(recipient, privateKey, writer(out), reason);

            if (false == rc) {
                throw std::runtime_error{"Failed to blind payment code"};
            }

            return out;
        }();

        auto output = UnallocatedVector<Space>{};
        constexpr auto size = sizeof(paymentcode::BinaryPreimage_3::key_);
        {
            auto& A = output.emplace_back(space(size));
            const auto rc =
                copy(privateKey.PublicKey(), preallocated(A.size(), A.data()));

            if (false == rc) {
                throw std::runtime_error{"Failed to copy public key"};
            }
        }

        switch (version_) {
            case 1:
            case 2: {
                generate_elements_v1(recipient, blind, output);
            } break;
            case 3:
            default: {
                generate_elements_v3(recipient, blind, output);
            }
        }

        return output;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto PaymentCode::generate_elements_v1(
    const opentxs::PaymentCode& recipient,
    const Space& blind,
    UnallocatedVector<Space>& output) const noexcept(false) -> void
{
    constexpr auto size = 65_uz;

    OT_ASSERT(blind.size() == sizeof(paymentcode::BinaryPreimage));

    const auto* b = blind.data();
    {
        auto& F = output.emplace_back(space(size));
        auto* i = F.data();
        *i = std::byte{0x04};
        std::advance(i, 1);
        const auto rc = recipient.Locator(preallocated(32, i), version_);
        std::advance(i, 32);

        if (false == rc) { throw std::runtime_error{"Failed to copy locator"}; }
        std::memcpy(i, b, 32);
        std::advance(b, 32);
    }
    {
        auto& G = output.emplace_back(space(size));
        auto* i = G.data();
        *i = std::byte{0x04};
        std::advance(i, 1);
        std::memcpy(i, b, 48);
        std::advance(i, 48);
        api_.Crypto().Util().RandomizeMemory(i, 16);
    }
}

auto PaymentCode::generate_elements_v3(
    const opentxs::PaymentCode& recipient,
    const Space& blind,
    UnallocatedVector<Space>& output) const noexcept(false) -> void
{
    constexpr auto size = sizeof(paymentcode::BinaryPreimage_3::key_);

    OT_ASSERT(blind.size() == size);

    {
        auto& F = output.emplace_back(space(size));
        auto* i = F.data();
        *i = std::byte{0x02};
        std::advance(i, 1);
        const auto rc = recipient.Locator(preallocated(size - 1, i), version_);

        if (false == rc) { throw std::runtime_error{"Failed to copy locator"}; }
    }
    {
        // G
        output.emplace_back(blind.begin(), blind.end());
    }
}

auto PaymentCode::Incoming(
    const opentxs::PaymentCode& sender,
    const Bip32Index index,
    const blockchain::Type chain,
    const opentxs::PasswordPrompt& reason,
    const std::uint8_t version) const noexcept
    -> crypto::asymmetric::key::EllipticCurve
{
    try {
        const auto effective = effective_version(version);
        const auto [localPrivate, remotePublic] =
            derive_keys(sender, index, 0, reason);

        switch (effective) {
            case 1:
            case 2: {
                const auto secret = shared_secret_payment_v1(
                    localPrivate, remotePublic, reason);

                return localPrivate.IncrementPrivate(secret, reason);
            }
            case 3: {
                const auto secret = shared_secret_payment_v3(
                    localPrivate, remotePublic, chain, reason);

                return localPrivate.IncrementPrivate(secret, reason);
            }
            default: {
                const auto error = UnallocatedCString{"Unsupported version "} +
                                   std::to_string(effective);

                throw std::runtime_error{error};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto PaymentCode::Key() const noexcept -> const crypto::asymmetric::key::HD&
{
    return key_;
}

auto PaymentCode::Locator(Writer&& dest, const std::uint8_t version)
    const noexcept -> bool
{
    try {
        switch (version_) {
            case 1: {
                const auto error =
                    UnallocatedCString{"Locator not defined for version "} +
                    std::to_string(version_);

                throw std::runtime_error(error);
            }
            case 2: {
                const auto pre = [&] {
                    auto out = std::array<std::byte, 33>{};
                    out[0] = std::byte{0x02};
                    const auto hashed = api_.Crypto().Hash().Digest(
                        crypto::HashType::Sha256,
                        binary_preimage(),
                        preallocated(out.size() - 1, std::next(out.data(), 1)));

                    if (false == hashed) {
                        throw std::runtime_error(
                            "Failed to calculate version 2 hash");
                    }

                    return out;
                }();

                if (false == copy(reader(pre), std::move(dest))) {

                    throw std::runtime_error("Failed to copy locator");
                }
            } break;
            case 3:
            default: {
                const auto effective = (0 == version) ? version_ : version;
                auto hash = space(64);
                auto rc = api_.Crypto().Hash().HMAC(
                    crypto::HashType::Sha512,
                    chain_code_.Bytes(),
                    ReadView{
                        reinterpret_cast<const char*>(&effective),
                        sizeof(effective)},
                    writer(hash));

                if (false == rc) {
                    throw std::runtime_error("Failed to hash locator");
                }

                if (false == copy(reader(hash), std::move(dest), 32)) {
                    throw std::runtime_error("Failed to copy locator");
                }
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }

    return true;
}

auto PaymentCode::match_locator(
    const std::uint8_t version,
    const Space& element) const noexcept(false) -> bool
{
    if (sizeof(paymentcode::BinaryPreimage_3::key_) > element.size()) {
        throw std::runtime_error{"Invalid F"};
    }

    const auto id = [&] {
        auto out = Space{};

        if (false == Locator(writer(out), version)) {
            throw std::runtime_error{"Failed to calculate locator"};
        }

        return out;
    }();

    OT_ASSERT(id.size() < element.size());

    return 0 == std::memcmp(std::next(element.data()), id.data(), id.size());
}

auto PaymentCode::Outgoing(
    const opentxs::PaymentCode& recipient,
    const Bip32Index index,
    const blockchain::Type chain,
    const opentxs::PasswordPrompt& reason,
    const std::uint8_t version) const noexcept
    -> crypto::asymmetric::key::EllipticCurve
{
    try {
        if (false == recipient.Key().IsValid()) {
            throw std::runtime_error{"remote payment code key is invalid"};
        }

        if (id_ == recipient.ID()) {
            throw std::runtime_error{"remote payment code must be different "
                                     "than local payment code"};
        }

        if (false == key_.HasPrivate()) {
            throw std::runtime_error{"Private key missing"};
        }

        const auto effective = effective_version(version, recipient.Version());
        const auto [localPrivate, remotePublic] =
            derive_keys(recipient, 0, index, reason);

        switch (effective) {
            case 1:
            case 2: {
                const auto secret = shared_secret_payment_v1(
                    localPrivate, remotePublic, reason);

                return remotePublic.IncrementPublic(secret);
            }
            case 3: {
                const auto secret = shared_secret_payment_v3(
                    localPrivate, remotePublic, chain, reason);

                return remotePublic.IncrementPublic(secret);
            }
            default: {
                const auto error = UnallocatedCString{"Unsupported version "} +
                                   std::to_string(effective);

                throw std::runtime_error{error};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto PaymentCode::postprocess(const Secret& in) const noexcept(false) -> Secret
{
    auto output = api_.Factory().Secret({});
    auto rc = api_.Crypto().Hash().Digest(
        crypto::HashType::Sha256, in.Bytes(), output.WriteInto());

    if (false == rc) {
        throw std::runtime_error{"Failed to hash shared secret"};
    }

    return output;
}

auto PaymentCode::Serialize(Writer&& destination) const noexcept -> bool
{
    auto serialized = proto::PaymentCode{};
    if (false == Serialize(serialized)) { return false; }

    return write(serialized, std::move(destination));
}

auto PaymentCode::Serialize(Serialized& output) const noexcept -> bool
{
    const auto key = pubkey_.Bytes();
    const auto code = chain_code_.Bytes();
    output.set_version(version_);
    output.set_key(key.data(), key.size());
    output.set_chaincode(code.data(), code.size());
    output.set_bitmessageversion(bitmessage_version_);
    output.set_bitmessagestream(bitmessage_stream_);

    return true;
}

auto PaymentCode::shared_secret_mask_v1(
    const crypto::asymmetric::key::EllipticCurve& local,
    const crypto::asymmetric::key::EllipticCurve& remote,
    const opentxs::PasswordPrompt& reason) const noexcept(false) -> Secret
{
    auto output = api_.Factory().Secret(0);
    auto rc = local.Internal().Provider().SharedSecret(
        remote.PublicKey(),
        local.PrivateKey(reason),
        crypto::SecretStyle::X_only,
        output);

    if (false == rc) {
        throw std::runtime_error{"Failed to calculate shared secret"};
    }

    return output;
}

auto PaymentCode::shared_secret_payment_v1(
    const crypto::asymmetric::key::EllipticCurve& local,
    const crypto::asymmetric::key::EllipticCurve& remote,
    const opentxs::PasswordPrompt& reason) const noexcept(false) -> Secret
{
    auto secret = shared_secret_mask_v1(local, remote, reason);

    return postprocess(secret);
}

auto PaymentCode::shared_secret_payment_v3(
    const crypto::asymmetric::key::EllipticCurve& local,
    const crypto::asymmetric::key::EllipticCurve& remote,
    const blockchain::Type chain,
    const opentxs::PasswordPrompt& reason) const noexcept(false) -> Secret
{
    auto secret = shared_secret_mask_v1(local, remote, reason);
    auto hmac = api_.Factory().Secret({});
    const auto bip44 = be::big_uint32_buf_t{
        static_cast<std::uint32_t>(blockchain::params::get(chain).Bip44Code())};

    static_assert(sizeof(bip44) == sizeof(std::uint32_t));

    auto rc = api_.Crypto().Hash().HMAC(
        crypto::HashType::Sha512,
        secret.Bytes(),
        ReadView{reinterpret_cast<const char*>(&bip44), sizeof(bip44)},
        hmac.WriteInto());

    if (false == rc) {
        throw std::runtime_error{"Failed to calculate shared secret hmac"};
    }

    return postprocess(hmac);
}

auto PaymentCode::Sign(
    const identity::credential::Base& credential,
    proto::Signature& sig,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    auto serialized = proto::Credential{};

    if (false ==
        credential.Internal().Serialize(
            serialized, identity::AS_PUBLIC, identity::WITHOUT_SIGNATURES)) {
        return false;
    }

    auto& signature = *serialized.add_signature();
    const bool output = key_.Internal().Sign(
        [&]() -> UnallocatedCString { return proto::ToString(serialized); },
        crypto::SignatureRole::NymIDSource,
        signature,
        ID(),
        reason);
    sig.CopyFrom(signature);

    return output;
}

auto PaymentCode::Sign(
    const opentxs::Data& data,
    opentxs::Data& output,
    const opentxs::PasswordPrompt& reason) const noexcept -> bool
{
    return key_.Internal().Provider().Sign(
        data.Bytes(),
        key_.PrivateKey(reason),
        crypto::HashType::Sha256,
        output.WriteInto());
}

auto PaymentCode::Unblind(
    const ReadView in,
    const crypto::asymmetric::key::EllipticCurve& remote,
    const ReadView outpoint,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> opentxs::PaymentCode
{
    try {
        if (2 < version_) {
            throw std::runtime_error{"Payment code version too high"};
        }

        if (false == key_.IsValid()) {
            throw std::runtime_error{"Missing private key"};
        }

        const auto local = key_.ChildKey(0, reason);

        if (false == local.IsValid()) {
            throw std::runtime_error{"Failed to derive notification key"};
        }

        const auto mask = calculate_mask_v1(local, remote, outpoint, reason);
        auto pre = [&] {
            auto out = paymentcode::BinaryPreimage{};

            if ((nullptr == in.data()) || (in.size() != sizeof(out))) {
                throw std::runtime_error{"Invalid blinded payment code"};
            }

            std::memcpy(reinterpret_cast<void*>(&out), in.data(), in.size());

            return out;
        }();
        apply_mask(mask, pre);

        return std::make_unique<PaymentCode>(
                   api_,
                   pre.version_,
                   pre.haveBitmessage(),
                   pre.xpub_.Key(),
                   pre.xpub_.Chaincode(),
                   pre.bm_version_,
                   pre.bm_stream_,
                   factory::Secp256k1Key(
                       api_,
                       local.Internal().asEllipticCurve().ECDSA(),
                       api_.Factory().Secret(0),
                       api_.Factory().SecretFromBytes(pre.xpub_.Chaincode()),
                       api_.Factory().DataFromBytes(pre.xpub_.Key()),
                       proto::HDPath{},
                       Bip32Fingerprint{},
                       crypto::asymmetric::Role::Sign,
                       crypto::asymmetric::key::EllipticCurve::DefaultVersion(),
                       reason,
                       {}  // TODO allocator
                       ))
            .release();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto PaymentCode::UnblindV3(
    const std::uint8_t version,
    const ReadView in,
    const crypto::asymmetric::key::EllipticCurve& remote,
    const opentxs::PasswordPrompt& reason) const noexcept
    -> opentxs::PaymentCode
{
    try {
        if (3 > version_) {
            throw std::runtime_error{"Local payment code version too low"};
        }

        if (false == key_.IsValid()) {
            throw std::runtime_error{"Missing private key"};
        }

        const auto local = key_.ChildKey(0, reason);

        if (false == local.IsValid()) {
            throw std::runtime_error{"Failed to derive notification key"};
        }

        const auto mask =
            calculate_mask_v3(local, remote, remote.PublicKey(), reason);
        const auto& ecdsa = local.Internal().asEllipticCurve().ECDSA();

        switch (version) {
            case 1:
            case 2: {
                return unblind_v1(in, mask, ecdsa, reason);
            }
            case 3:
            default: {
                return unblind_v3(version, in, mask, ecdsa, reason);
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto PaymentCode::unblind_v1(
    const ReadView in,
    const Mask& mask,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::PasswordPrompt& reason) const -> opentxs::PaymentCode
{
    const auto pre = [&] {
        auto out = paymentcode::BinaryPreimage{};

        if ((nullptr == in.data()) || (in.size() != (sizeof(out)))) {
            throw std::runtime_error{"Invalid blinded payment code (v1)"};
        }

        auto* i = reinterpret_cast<char*>(reinterpret_cast<void*>(&out));
        std::memcpy(i, in.data(), in.size());
        apply_mask(mask, out);

        return out;
    }();

    return std::make_unique<PaymentCode>(
               api_,
               pre.version_,
               pre.haveBitmessage(),
               pre.xpub_.Key(),
               pre.xpub_.Chaincode(),
               pre.bm_version_,
               pre.bm_stream_,
               factory::Secp256k1Key(
                   api_,
                   ecdsa,
                   api_.Factory().Secret(0),
                   api_.Factory().SecretFromBytes(pre.xpub_.Chaincode()),
                   api_.Factory().DataFromBytes(pre.xpub_.Key()),
                   proto::HDPath{},
                   Bip32Fingerprint{},
                   crypto::asymmetric::Role::Sign,
                   crypto::asymmetric::key::EllipticCurve::DefaultVersion(),
                   reason,
                   {}  // TODO allocator
                   ))
        .release();
}

auto PaymentCode::unblind_v3(
    const std::uint8_t version,
    const ReadView in,
    const Mask& mask,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::PasswordPrompt& reason) const -> opentxs::PaymentCode
{
    const auto pre = [&] {
        auto out = paymentcode::BinaryPreimage_3{};
        out.version_ = version;

        if ((nullptr == in.data()) || ((in.size() + 1u) != sizeof(out))) {
            throw std::runtime_error{"Invalid blinded payment code (v3)"};
        }

        auto* i = reinterpret_cast<char*>(reinterpret_cast<void*>(&out));
        std::memcpy(std::next(i), in.data(), in.size());
        apply_mask(mask, out);

        return out;
    }();
    const auto code = [&] {
        auto out = api_.Factory().Secret(0);
        const auto rc = api_.Crypto().Hash().Digest(
            crypto::HashType::Sha256D, pre.Key(), out.WriteInto());

        if (false == rc) {
            throw std::runtime_error{"Failed to calculate chain code"};
        }

        return out;
    }();

    return std::make_unique<PaymentCode>(
               api_,
               pre.version_,
               false,
               pre.Key(),
               code.Bytes(),
               0,
               0,
               factory::Secp256k1Key(
                   api_,
                   ecdsa,
                   api_.Factory().Secret(0),
                   code,
                   api_.Factory().DataFromBytes(pre.Key()),
                   proto::HDPath{},
                   Bip32Fingerprint{},
                   crypto::asymmetric::Role::Sign,
                   crypto::asymmetric::key::EllipticCurve::DefaultVersion(),
                   reason,
                   {}  // TODO allocator
                   ))
        .release();
}

auto PaymentCode::Valid() const noexcept -> bool
{
    if (0 == version_) { return false; }

    if (pubkey_size_ != pubkey_.size()) { return false; }

    if (chain_code_size_ != chain_code_.size()) { return false; }

    auto serialized = proto::PaymentCode{};

    if (false == Serialize(serialized)) { return false; }

    return proto::Validate<proto::PaymentCode>(serialized, SILENT);
}

auto PaymentCode::Verify(
    const proto::Credential& master,
    const proto::Signature& sourceSignature) const noexcept -> bool
{
    if (false == proto::Validate<proto::Credential>(
                     master,
                     VERBOSE,
                     proto::KEYMODE_PUBLIC,
                     proto::CREDROLE_MASTERKEY,
                     false)) {
        LogError()(OT_PRETTY_CLASS())("Invalid master credential syntax.")
            .Flush();

        return false;
    }

    const bool sameSource =
        (*this == master.masterdata().source().paymentcode());

    if (false == sameSource) {
        LogError()(OT_PRETTY_CLASS())(
            "Master credential was not derived from this source.")
            .Flush();

        return false;
    }

    auto copy = proto::Credential{};
    copy.CopyFrom(master);
    auto& signature = *copy.add_signature();
    signature.CopyFrom(sourceSignature);
    signature.clear_signature();

    return key_.Internal().Verify(
        api_.Factory().Internal().Data(copy), sourceSignature);
}

PaymentCode::~PaymentCode() = default;
}  // namespace opentxs::implementation
