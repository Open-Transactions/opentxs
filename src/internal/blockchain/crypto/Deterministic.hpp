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
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::internal
{
struct Deterministic : virtual public crypto::Deterministic,
                       virtual public Subaccount {
    auto Floor(const Subchain type) const noexcept
        -> std::optional<Bip32Index> override;
    auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index> override;
    auto InternalDeterministic() const noexcept
        -> internal::Deterministic& final
    {
        return const_cast<Deterministic&>(*this);
    }
    auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& override;
    auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> override;
    auto Lookahead() const noexcept -> std::size_t override;
    auto Path() const noexcept -> proto::HDPath override;
    auto PathRoot() const noexcept -> const opentxs::crypto::SeedID& override;
    auto Reserve(
        const Subchain type,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index> override;
    auto Reserve(
        const Subchain type,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index> override;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch override;
    auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch override;
    auto RootNode(const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::HD& override;
};
}  // namespace opentxs::blockchain::crypto::internal
