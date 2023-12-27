// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
class Subaccount;
}  // namespace internal

class HD;
class PaymentCode;
}  // namespace crypto
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Deterministic : public Subaccount
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Deterministic&;

    auto asHD() const noexcept -> const crypto::HD&;
    auto asPaymentCode() const noexcept -> const crypto::PaymentCode&;
    auto Floor(const Subchain type) const noexcept -> std::optional<Bip32Index>;
    auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index>;
    auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&;
    auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index>;
    auto Lookahead() const noexcept -> std::size_t;
    auto PathRoot() const noexcept -> const opentxs::crypto::SeedID&;
    auto Reserve(
        const Subchain type,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index>;
    auto Reserve(
        const Subchain type,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index>;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch;
    auto RootNode(const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::HD&;

    auto asHD() noexcept -> crypto::HD&;
    auto asPaymentCode() noexcept -> crypto::PaymentCode&;

    OPENTXS_NO_EXPORT Deterministic(
        std::shared_ptr<internal::Subaccount> imp) noexcept;
    Deterministic() noexcept = delete;
    Deterministic(const Deterministic& rhs) noexcept;
    Deterministic(Deterministic&& rhs) noexcept;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;

    ~Deterministic() override;
};
}  // namespace opentxs::blockchain::crypto
