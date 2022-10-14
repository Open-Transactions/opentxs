// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <AsymmetricKey.pb.h>
#include <memory>

#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/library/Null.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/RSA.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"

namespace opentxs::crypto::key::blank
{
class Keypair final : virtual public key::Keypair
{
public:
    operator bool() const noexcept final { return false; }

    auto CheckCapability(const identity::NymCapability&) const noexcept
        -> bool final
    {
        return {};
    }
    auto GetPrivateKey() const noexcept(false) -> const Asymmetric& final
    {
        throw std::runtime_error("");
    }
    auto GetPublicKey() const noexcept(false) -> const Asymmetric& final
    {
        throw std::runtime_error("");
    }
    auto GetPublicKeyBySignature(Keys&, const Signature&, bool) const noexcept
        -> std::int32_t final
    {
        return {};
    }
    auto Serialize(proto::AsymmetricKey& serialized, bool) const noexcept
        -> bool final
    {
        serialized = {};
        return {};
    }
    auto GetTransportKey(Data&, Secret&, const PasswordPrompt&) const noexcept
        -> bool final
    {
        return {};
    }

    auto clone() const -> Keypair* final { return new Keypair; }

    ~Keypair() final = default;
};

class Asymmetric : virtual public key::Asymmetric
{
public:
    auto asPublic() const noexcept -> std::unique_ptr<key::Asymmetric> final
    {
        return {};
    }
    auto CalculateHash(const crypto::HashType, const PasswordPrompt&)
        const noexcept -> ByteArray final
    {
        return ByteArray{};
    }
    auto CalculateID(identifier::Generic&) const noexcept -> bool final
    {
        return false;
    }
    auto CalculateTag(
        const identity::Authority&,
        const crypto::key::asymmetric::Algorithm,
        const PasswordPrompt&,
        std::uint32_t&,
        Secret&) const noexcept -> bool final
    {
        return false;
    }
    auto CalculateTag(
        const key::Asymmetric&,
        const identifier::Generic&,
        const PasswordPrompt&,
        std::uint32_t&) const noexcept -> bool final
    {
        return false;
    }
    auto CalculateSessionPassword(
        const key::Asymmetric&,
        const PasswordPrompt&,
        Secret&) const noexcept -> bool final
    {
        return false;
    }
    auto engine() const noexcept
        -> const opentxs::crypto::AsymmetricProvider& final
    {
        static const auto provider = crypto::blank::AsymmetricProvider{};

        return provider;
    }
    auto GetMetadata() const noexcept -> const OTSignatureMetadata* final
    {
        return nullptr;
    }
    auto hasCapability(const identity::NymCapability&) const noexcept
        -> bool final
    {
        return false;
    }
    auto HasPrivate() const noexcept -> bool final { return false; }
    auto HasPublic() const noexcept -> bool final { return false; }
    auto keyType() const noexcept -> crypto::key::asymmetric::Algorithm final
    {
        return crypto::key::asymmetric::Algorithm::Null;
    }
    auto Params() const noexcept -> ReadView final { return {}; }
    auto Path() const noexcept -> const UnallocatedCString final { return {}; }
    auto Path(proto::HDPath&) const noexcept -> bool final { return false; }
    auto PrivateKey(const PasswordPrompt&) const noexcept -> ReadView final
    {
        return {};
    }
    auto PublicKey() const noexcept -> ReadView final { return {}; }
    auto Role() const noexcept -> opentxs::crypto::key::asymmetric::Role final
    {
        return {};
    }
    auto Serialize(Serialized& serialized) const noexcept -> bool final
    {
        serialized = Serialized{};

        return true;
    }
    auto SigHashType() const noexcept -> crypto::HashType final
    {
        return crypto::HashType::None;
    }
    auto Sign(
        const GetPreimage,
        const crypto::SignatureRole,
        proto::Signature&,
        const identifier::Generic&,
        const PasswordPrompt&,
        const crypto::HashType) const noexcept -> bool final
    {
        return false;
    }
    auto Sign(
        const ReadView,
        const crypto::HashType,
        const AllocateOutput,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return false;
    }
    auto TransportKey(Data&, Secret&, const PasswordPrompt&) const noexcept
        -> bool final
    {
        return false;
    }
    auto Verify(const Data&, const proto::Signature&) const noexcept
        -> bool final
    {
        return false;
    }
    auto Version() const noexcept -> VersionNumber final { return {}; }

    operator bool() const noexcept override { return false; }
    auto operator==(const proto::AsymmetricKey&) const noexcept -> bool final
    {
        return false;
    }

    Asymmetric() = default;
    ~Asymmetric() override = default;

private:
    auto clone() const noexcept -> Asymmetric* override
    {
        return new Asymmetric;
    }
};

class EllipticCurve : virtual public key::EllipticCurve, public Asymmetric
{
public:
    operator bool() const noexcept final { return false; }

    auto asPublicEC() const noexcept
        -> std::unique_ptr<key::EllipticCurve> final
    {
        return {};
    }
    auto CloneEC() const noexcept -> std::unique_ptr<key::EllipticCurve> final
    {
        return {};
    }
    auto ECDSA() const noexcept -> const opentxs::crypto::EcdsaProvider& final
    {
        static const auto provider = crypto::blank::EcdsaProvider{};

        return provider;
    }
    auto IncrementPrivate(const Secret&, const PasswordPrompt&) const noexcept
        -> std::unique_ptr<key::EllipticCurve> final
    {
        return {};
    }
    auto IncrementPublic(const Secret&) const noexcept
        -> std::unique_ptr<key::EllipticCurve> final
    {
        return {};
    }
    auto SignDER(
        const ReadView,
        const crypto::HashType,
        Space&,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return false;
    }

    EllipticCurve() = default;
    ~EllipticCurve() override = default;
};

class HD : virtual public key::HD, public EllipticCurve
{
public:
    auto Chaincode(const PasswordPrompt&) const noexcept -> ReadView final
    {
        return {};
    }
    auto ChildKey(const Bip32Index, const PasswordPrompt&) const noexcept
        -> std::unique_ptr<key::HD> final
    {
        return {};
    }
    auto Depth() const noexcept -> int final { return {}; }
    auto Fingerprint() const noexcept -> Bip32Fingerprint final { return {}; }
    auto Parent() const noexcept -> Bip32Fingerprint final { return {}; }
    auto Xprv(const PasswordPrompt&) const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto Xpub(const PasswordPrompt&) const noexcept -> UnallocatedCString final
    {
        return {};
    }

    HD() = default;
    ~HD() override = default;

private:
    auto clone() const noexcept -> HD* override { return new HD; }
};

class Ed25519 final : virtual public key::Ed25519, public HD
{
public:
    Ed25519() = default;
    ~Ed25519() final = default;

private:
    auto clone() const noexcept -> Ed25519* final { return new Ed25519; }
};

class RSA final : virtual public key::RSA, public Asymmetric
{
public:
    RSA() = default;
    ~RSA() final = default;

private:
    auto clone() const noexcept -> RSA* final { return new RSA; }
};

class Secp256k1 final : virtual public key::Secp256k1, public HD
{
public:
    Secp256k1() = default;
    ~Secp256k1() final = default;

private:
    auto clone() const noexcept -> Secp256k1* final { return new Secp256k1; }
};
}  // namespace opentxs::crypto::key::blank
