// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <future>
#include <set>
#include <string>

#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace identifier
{
class Nym;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs

namespace opentxs::otx::client
{
class Pair
{
public:
    virtual auto AddIssuer(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const std::string& pairingCode) const noexcept -> bool = 0;
    virtual auto CheckIssuer(
        const identifier::Nym& localNymID,
        const identifier::UnitDefinition& unitDefinitionID) const noexcept
        -> bool = 0;
    virtual auto IssuerDetails(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID) const noexcept -> std::string = 0;
    virtual auto IssuerList(
        const identifier::Nym& localNymID,
        const bool onlyTrusted) const noexcept -> std::pmr::set<OTNymID> = 0;
    /** For unit tests */
    virtual auto Stop() const noexcept -> std::shared_future<void> = 0;
    /** For unit tests */
    virtual auto Wait() const noexcept -> std::shared_future<void> = 0;

    virtual auto init() noexcept -> void = 0;

    virtual ~Pair() = default;

protected:
    Pair() noexcept = default;

private:
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    auto operator=(const Pair&) -> Pair& = delete;
    auto operator=(Pair&&) -> Pair& = delete;
};
}  // namespace opentxs::otx::client
