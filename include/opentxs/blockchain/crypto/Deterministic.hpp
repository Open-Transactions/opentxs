// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Deterministic;
}  // namespace internal
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

namespace proto
{
class HDPath;
}  // namespace proto

namespace identifier
{
class Generic;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto
{
class OPENTXS_EXPORT Deterministic : virtual public Subaccount
{
public:
    using Batch = UnallocatedVector<Bip32Index>;

    virtual auto Floor(const Subchain type) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index> = 0;
    OPENTXS_NO_EXPORT virtual auto InternalDeterministic() const noexcept
        -> internal::Deterministic& = 0;
    virtual auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> const opentxs::crypto::asymmetric::key::EllipticCurve& = 0;
    virtual auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto Lookahead() const noexcept -> std::size_t = 0;
    OPENTXS_NO_EXPORT virtual auto Path() const noexcept -> proto::HDPath = 0;
    virtual auto PathRoot() const noexcept -> const UnallocatedCString = 0;
    virtual auto Reserve(
        const Subchain type,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto Reserve(
        const Subchain type,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch = 0;
    virtual auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const PasswordPrompt& reason,
        const std::string_view label = {},
        const Time time = Clock::now()) const noexcept -> Batch = 0;
    virtual auto RootNode(const PasswordPrompt& reason) const noexcept
        -> const opentxs::crypto::asymmetric::key::HD& = 0;

    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;

    OPENTXS_NO_EXPORT ~Deterministic() override = default;

protected:
    Deterministic() noexcept = default;
};
}  // namespace opentxs::blockchain::crypto
