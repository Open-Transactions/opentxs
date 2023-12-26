// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <HDPath.pb.h>
#include <cstddef>
#include <optional>
#include <string_view>

#include "internal/blockchain/crypto/Subaccount.hpp"
#include "opentxs/Time.hpp"
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
class HD;
class PaymentCode;
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

namespace opentxs::blockchain::crypto::internal
{
class Deterministic : virtual public Subaccount
{
public:
    static auto Blank() noexcept -> Deterministic&;

    virtual auto asHD() const noexcept -> const internal::HD&;
    virtual auto asHDPublic() const noexcept -> const crypto::HD&;
    virtual auto asPaymentCode() const noexcept -> const internal::PaymentCode&;
    virtual auto asPaymentCodePublic() const noexcept
        -> const crypto::PaymentCode&;
    virtual auto Floor(const Subchain type) const noexcept
        -> std::optional<Bip32Index>;
    virtual auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index>;
    virtual auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve&;
    virtual auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index>;
    virtual auto Lookahead() const noexcept -> std::size_t;
    virtual auto Path() const noexcept -> proto::HDPath;
    virtual auto PathRoot() const noexcept -> const opentxs::crypto::SeedID&;
    virtual auto Reserve(
        const Subchain type,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index>;
    virtual auto Reserve(
        const Subchain type,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index>;
    virtual auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch;
    virtual auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch;
    virtual auto RootNode(const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::HD&;

    virtual auto asHD() noexcept -> internal::HD&;
    virtual auto asHDPublic() noexcept -> crypto::HD&;
    virtual auto asPaymentCode() noexcept -> internal::PaymentCode&;
    virtual auto asPaymentCodePublic() noexcept -> crypto::PaymentCode&;

    Deterministic() = default;
    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;

    ~Deterministic() override = default;
};
}  // namespace opentxs::blockchain::crypto::internal
