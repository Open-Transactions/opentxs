// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/SymmetricProvider.hpp"

namespace opentxs::crypto::blank
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    auto RandomKeypair(
        const AllocateOutput,
        const AllocateOutput,
        const AllocateOutput) const noexcept -> bool final
    {
        return false;
    }
    auto RandomKeypair(
        const AllocateOutput,
        const AllocateOutput,
        const Parameters&,
        const AllocateOutput) const noexcept -> bool final
    {
        return false;
    }
    auto RandomKeypair(
        const AllocateOutput,
        const AllocateOutput,
        const opentxs::crypto::key::asymmetric::Role,
        const AllocateOutput) const noexcept -> bool final
    {
        return false;
    }
    auto RandomKeypair(
        const AllocateOutput,
        const AllocateOutput,
        const opentxs::crypto::key::asymmetric::Role,
        const Parameters&,
        const AllocateOutput) const noexcept -> bool final
    {
        return false;
    }
    auto SeedToCurveKey(
        const ReadView,
        const AllocateOutput,
        const AllocateOutput) const noexcept -> bool final
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
    auto Sign(
        const ReadView,
        const ReadView,
        const crypto::HashType,
        const AllocateOutput) const -> bool final
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
    auto PubkeyAdd(const ReadView, const ReadView, const AllocateOutput)
        const noexcept -> bool final
    {
        return false;
    }
    auto ScalarAdd(const ReadView, const ReadView, const AllocateOutput)
        const noexcept -> bool final
    {
        return false;
    }
    auto ScalarMultiplyBase(const ReadView, const AllocateOutput) const noexcept
        -> bool final
    {
        return false;
    }
    auto SignDER(const ReadView, const ReadView, const crypto::HashType, Space&)
        const noexcept -> bool final
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
    auto DefaultMode() const -> opentxs::crypto::key::symmetric::Algorithm final
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
        const crypto::key::symmetric::Source,
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
    auto IvSize(const opentxs::crypto::key::symmetric::Algorithm) const
        -> std::size_t final
    {
        return {};
    }
    auto KeySize(const opentxs::crypto::key::symmetric::Algorithm) const
        -> std::size_t final
    {
        return {};
    }
    auto SaltSize(const crypto::key::symmetric::Source) const
        -> std::size_t final
    {
        return {};
    }
    auto TagSize(const opentxs::crypto::key::symmetric::Algorithm) const
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
