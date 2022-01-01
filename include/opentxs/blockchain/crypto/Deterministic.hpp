// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <optional>
#include <string>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/core/identifier/Generic.hpp"

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
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Deterministic : virtual public Subaccount
{
public:
    using Batch = std::pmr::vector<Bip32Index>;

    virtual auto Floor(const Subchain type) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index> = 0;
    OPENTXS_NO_EXPORT virtual auto InternalDeterministic() const noexcept
        -> internal::Deterministic& = 0;
    virtual auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> ECKey = 0;
    virtual auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto Lookahead() const noexcept -> std::size_t = 0;
    OPENTXS_NO_EXPORT virtual auto Path() const noexcept -> proto::HDPath = 0;
    virtual auto PathRoot() const noexcept -> const std::string = 0;
    virtual auto Reserve(
        const Subchain type,
        const PasswordPrompt& reason,
        const Identifier& contact = Identifier::Factory(),
        const std::string& label = {},
        const Time time = Clock::now()) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto Reserve(
        const Subchain type,
        const std::size_t batch,
        const PasswordPrompt& reason,
        const Identifier& contact = Identifier::Factory(),
        const std::string& label = {},
        const Time time = Clock::now()) const noexcept -> Batch = 0;
    virtual auto RootNode(const PasswordPrompt& reason) const noexcept
        -> HDKey = 0;

    OPENTXS_NO_EXPORT ~Deterministic() override = default;

protected:
    Deterministic() noexcept = default;

private:
    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
