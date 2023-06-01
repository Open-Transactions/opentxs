// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/SymmetricProvider.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Factory;
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::blank
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    auto RandomKeypair(Writer&&, Writer&&, Writer&&) const noexcept
        -> bool final
    {
        return false;
    }
    auto RandomKeypair(Writer&&, Writer&&, const Parameters&, Writer&&)
        const noexcept -> bool final
    {
        return false;
    }
    auto RandomKeypair(
        Writer&&,
        Writer&&,
        const opentxs::crypto::asymmetric::Role,
        Writer&&) const noexcept -> bool final
    {
        return false;
    }
    auto RandomKeypair(
        Writer&&,
        Writer&&,
        const opentxs::crypto::asymmetric::Role,
        const Parameters&,
        Writer&&) const noexcept -> bool final
    {
        return false;
    }
    auto SeedToCurveKey(const ReadView, Writer&&, Writer&&) const noexcept
        -> bool final
    {
        return false;
    }
    auto SharedSecret(
        const ReadView,
        const ReadView,
        const SecretStyle,
        Secret&) const noexcept -> bool final
    {
        return false;
    }
    auto Sign(const ReadView, const ReadView, const crypto::HashType, Writer&&)
        const -> bool final
    {
        return false;
    }
    auto SignContract(
        const api::Session&,
        const String&,
        const ReadView,
        const crypto::HashType,
        Signature&) const -> bool final
    {
        return false;
    }
    auto Verify(
        const ReadView,
        const ReadView,
        const ReadView,
        const crypto::HashType) const -> bool final
    {
        return false;
    }
    auto VerifyContractSignature(
        const api::Session&,
        const String&,
        const ReadView,
        const Signature&,
        const crypto::HashType) const -> bool final
    {
        return false;
    }

    auto Init(const std::shared_ptr<const api::Factory>&) noexcept -> void final
    {
    }

    AsymmetricProvider() = default;
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;

    ~AsymmetricProvider() override = default;
};

class EcdsaProvider final : public crypto::EcdsaProvider,
                            public AsymmetricProvider
{
public:
    auto PubkeyAdd(ReadView, ReadView, Writer&&) const noexcept -> bool final
    {
        return false;
    }
    auto ScalarAdd(ReadView, ReadView, Writer&&) const noexcept -> bool final
    {
        return false;
    }
    auto ScalarMultiplyBase(ReadView, Writer&&) const noexcept -> bool final
    {
        return false;
    }
    auto SignDER(ReadView, ReadView, crypto::HashType, Writer&&) const noexcept
        -> bool final
    {
        return false;
    }

    EcdsaProvider() = default;
    EcdsaProvider(const EcdsaProvider&) = delete;
    EcdsaProvider(EcdsaProvider&&) = delete;
    auto operator=(const EcdsaProvider&) -> EcdsaProvider& = delete;
    auto operator=(EcdsaProvider&&) -> EcdsaProvider& = delete;

    ~EcdsaProvider() final = default;
};

class SymmetricProvider final : public crypto::SymmetricProvider
{
public:
    auto Decrypt(
        const proto::Ciphertext&,
        const std::uint8_t*,
        const std::size_t,
        std::uint8_t*) const -> bool final
    {
        return {};
    }
    auto DefaultMode() const -> opentxs::crypto::symmetric::Algorithm final
    {
        return {};
    }
    auto Derive(
        const std::uint8_t*,
        const std::size_t,
        const std::uint8_t*,
        const std::size_t,
        const std::uint64_t,
        const std::uint64_t,
        const std::uint64_t,
        const crypto::symmetric::Source,
        std::uint8_t*,
        std::size_t) const -> bool final
    {
        return {};
    }
    auto Encrypt(
        const std::uint8_t*,
        const std::size_t,
        const std::uint8_t*,
        const std::size_t,
        proto::Ciphertext&) const -> bool final
    {
        return {};
    }
    auto IvSize(const opentxs::crypto::symmetric::Algorithm) const
        -> std::size_t final
    {
        return {};
    }
    auto KeySize(const opentxs::crypto::symmetric::Algorithm) const
        -> std::size_t final
    {
        return {};
    }
    auto SaltSize(const crypto::symmetric::Source) const -> std::size_t final
    {
        return {};
    }
    auto TagSize(const opentxs::crypto::symmetric::Algorithm) const
        -> std::size_t final
    {
        return {};
    }

    SymmetricProvider() = default;
    SymmetricProvider(const SymmetricProvider&) = delete;
    SymmetricProvider(SymmetricProvider&&) = delete;
    auto operator=(const SymmetricProvider&) -> SymmetricProvider& = delete;
    auto operator=(SymmetricProvider&&) -> SymmetricProvider& = delete;

    ~SymmetricProvider() final = default;
};
}  // namespace opentxs::crypto::blank
