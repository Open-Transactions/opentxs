// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::Language
// IWYU pragma: no_forward_declare opentxs::crypto::ParameterType
// IWYU pragma: no_forward_declare opentxs::crypto::SeedStrength
// IWYU pragma: no_forward_declare opentxs::crypto::SeedStyle
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Algorithm
// IWYU pragma: no_forward_declare opentxs::identity::CredentialType
// IWYU pragma: no_forward_declare opentxs::identity::SourceProofType
// IWYU pragma: no_forward_declare opentxs::identity::SourceType
// IWYU pragma: no_include "opentxs/crypto/Language.hpp"
// IWYU pragma: no_include "opentxs/crypto/ParameterType.hpp"
// IWYU pragma: no_include "opentxs/crypto/SeedStrength.hpp"
// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Algorithm.hpp"
// IWYU pragma: no_include "opentxs/identity/CredentialType.hpp"
// IWYU pragma: no_include "opentxs/identity/SourceProofType.hpp"
// IWYU pragma: no_include "opentxs/identity/SourceType.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
namespace internal
{
class Parameters;
}  // namespace internal

class Parameters;
}  // namespace crypto

class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct hash<opentxs::crypto::Parameters> {
    auto operator()(const opentxs::crypto::Parameters& rhs) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs::crypto
{
class OPENTXS_EXPORT Parameters
{
public:
    class Imp;

    static auto DefaultCredential() noexcept -> identity::CredentialType;
    static auto DefaultSource() noexcept -> identity::SourceType;
    static auto DefaultType() noexcept -> ParameterType;

    auto Algorithm() const noexcept -> asymmetric::Algorithm;
    auto ChangeType(const ParameterType type) const noexcept -> Parameters;
    auto credentialType() const noexcept -> identity::CredentialType;
    auto CredIndex() const noexcept -> Bip32Index;
    auto Credset() const noexcept -> Bip32Index;
    auto Default() const noexcept -> bool;
    auto DHParams() const noexcept -> ReadView;
    auto Entropy() const noexcept -> const Secret&;
    auto keySize() const noexcept -> std::int32_t;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Parameters&;
    auto Nym() const noexcept -> Bip32Index;
    auto nymParameterType() const noexcept -> ParameterType;
    auto PaymentCodeVersion() const noexcept -> std::uint8_t;
    auto Seed() const noexcept -> UnallocatedCString;
    auto SeedLanguage() const noexcept -> Language;
    auto SeedStrength() const noexcept -> crypto::SeedStrength;
    auto SeedStyle() const noexcept -> crypto::SeedStyle;
    auto SourceProofType() const noexcept -> identity::SourceProofType;
    auto SourceType() const noexcept -> identity::SourceType;
    auto UseAutoIndex() const noexcept -> bool;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Parameters&;
    auto SetCredIndex(const Bip32Index path) noexcept -> void;
    auto SetCredset(const Bip32Index path) noexcept -> void;
    auto SetDefault(const bool in) noexcept -> void;
    auto SetEntropy(const Secret& entropy) noexcept -> void;
    auto setKeySize(std::int32_t keySize) noexcept -> void;
    auto SetNym(const Bip32Index path) noexcept -> void;
    auto SetDHParams(const ReadView bytes) noexcept -> void;
    auto SetPaymentCodeVersion(const std::uint8_t version) noexcept -> void;
    auto SetSeed(const UnallocatedCString& seed) noexcept -> void;
    auto SetSeedLanguage(const Language lang) noexcept -> void;
    auto SetSeedStrength(const crypto::SeedStrength value) noexcept -> void;
    auto SetSeedStyle(const crypto::SeedStyle type) noexcept -> void;
    auto SetUseAutoIndex(const bool use) noexcept -> void;
    auto swap(Parameters& rhs) noexcept -> void;

    Parameters(
        const ParameterType type = DefaultType(),
        const identity::CredentialType credential = DefaultCredential(),
        const identity::SourceType source = DefaultSource(),
        const std::uint8_t pcVersion = 0) noexcept;
    Parameters(
        asymmetric::Algorithm key,
        identity::CredentialType credential = DefaultCredential(),
        const identity::SourceType source = DefaultSource(),
        const std::uint8_t pcVersion = 0) noexcept;
    Parameters(const std::int32_t keySize) noexcept;
    Parameters(
        const UnallocatedCString& seedID,
        const int index,
        const std::uint8_t pcVersion = 0) noexcept;
    Parameters(const Parameters& rhs) noexcept;
    Parameters(Parameters&& rhs) noexcept;
    auto operator=(const Parameters&) noexcept -> Parameters&;
    auto operator=(Parameters&&) noexcept -> Parameters&;

    virtual ~Parameters();

private:
    Imp* imp_;
};

auto swap(Parameters& lhs, Parameters& rhs) noexcept -> void;
auto operator<(const Parameters& lhs, const Parameters& rhs) noexcept -> bool;
auto operator==(const Parameters& lhs, const Parameters& rhs) noexcept -> bool;
}  // namespace opentxs::crypto
