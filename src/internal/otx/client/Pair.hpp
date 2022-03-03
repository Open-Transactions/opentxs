// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "opentxs/Version.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identifier
{
class Nym;
class UnitDefinition;
}  // namespace identifier
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client
{
class Pair
{
public:
    virtual auto AddIssuer(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID,
        const UnallocatedCString& pairingCode) const noexcept -> bool = 0;
    virtual auto CheckIssuer(
        const identifier::Nym& localNymID,
        const identifier::UnitDefinition& unitDefinitionID) const noexcept
        -> bool = 0;
    virtual auto IssuerDetails(
        const identifier::Nym& localNymID,
        const identifier::Nym& issuerNymID) const noexcept
        -> UnallocatedCString = 0;
    virtual auto IssuerList(
        const identifier::Nym& localNymID,
        const bool onlyTrusted) const noexcept -> UnallocatedSet<OTNymID> = 0;
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
