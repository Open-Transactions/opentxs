// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/api/Crypto.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/OpenSSL.hpp"
#include "internal/crypto/library/Secp256k1.hpp"
#include "internal/crypto/library/Sodium.hpp"
#include "internal/crypto/library/SymmetricProvider.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/crypto/symmetric/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Factory;
}  // namespace internal

class Settings;
}  // namespace api

namespace crypto
{
namespace blank
{
class EcdsaProvider;
}  // namespace blank
}  // namespace crypto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
class Crypto final : public internal::Crypto
{
public:
    auto AsymmetricProvider(opentxs::crypto::asymmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::AsymmetricProvider& final;
    auto BIP32() const noexcept -> const opentxs::crypto::Bip32& final;
    auto BIP39() const noexcept -> const opentxs::crypto::Bip39& final;
    auto Config() const noexcept -> const crypto::Config& final;
    auto EllipticProvider(opentxs::crypto::asymmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::EcdsaProvider& final;
    auto Encode() const noexcept -> const crypto::Encode& final;
    auto Hash() const noexcept -> const crypto::Hash& final;
    auto Libsecp256k1() const noexcept
        -> const opentxs::crypto::Secp256k1& final;
    auto Libsodium() const noexcept -> const opentxs::crypto::Sodium& final;
    auto OpenSSL() const noexcept -> const opentxs::crypto::OpenSSL& final;
    auto SymmetricProvider(opentxs::crypto::symmetric::Algorithm type)
        const noexcept -> const opentxs::crypto::SymmetricProvider& final;
    auto SymmetricProvider(opentxs::crypto::symmetric::Source type)
        const noexcept -> const opentxs::crypto::SymmetricProvider& final;
    auto Util() const noexcept -> const crypto::Util& final;
    auto hasLibsecp256k1() const noexcept -> bool final;
    auto hasOpenSSL() const noexcept -> bool final;
    auto hasSodium() const noexcept -> bool final;

    auto Init(
        const std::shared_ptr<const api::internal::Factory>& factory) noexcept
        -> void final;

    Crypto(const api::Settings& settings) noexcept;
    Crypto() = delete;
    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    auto operator=(const Crypto&) -> Crypto& = delete;
    auto operator=(Crypto&&) -> Crypto& = delete;

    ~Crypto() final;

private:
    using AType = opentxs::crypto::asymmetric::Algorithm;
    using SaType = opentxs::crypto::symmetric::Algorithm;
    using SsType = opentxs::crypto::symmetric::Source;
    using AMap = UnallocatedMap<AType, opentxs::crypto::AsymmetricProvider*>;
    using EMap = UnallocatedMap<AType, opentxs::crypto::EcdsaProvider*>;
    using SaMap = UnallocatedMap<SaType, opentxs::crypto::SymmetricProvider*>;
    using SsMap = UnallocatedMap<SsType, opentxs::crypto::SymmetricProvider*>;

    static const opentxs::crypto::blank::EcdsaProvider blank_;

    std::unique_ptr<api::crypto::Config> config_;
    std::unique_ptr<opentxs::crypto::Sodium> sodium_;
    std::unique_ptr<opentxs::crypto::OpenSSL> ssl_;
    const api::crypto::Util& util_;
    std::unique_ptr<opentxs::crypto::Secp256k1> secp256k1_;
    std::unique_ptr<opentxs::crypto::Bip39> bip39_p_;
    opentxs::crypto::Bip32 bip32_;
    const opentxs::crypto::Bip39& bip39_;
    std::unique_ptr<crypto::Encode> encode_;
    std::unique_ptr<crypto::Hash> hash_;
    const AMap asymmetric_map_;
    const EMap elliptic_map_;
    const SaMap symmetric_algorithm_map_;
    const SsMap symmetric_source_map_;

    auto Init() noexcept -> void;
    auto Init_Libsecp256k1() noexcept -> void;
    auto Init_OpenSSL() noexcept -> void;
    auto Init_Sodium() noexcept -> void;
    auto Cleanup() noexcept -> void;
};
}  // namespace opentxs::api::imp
