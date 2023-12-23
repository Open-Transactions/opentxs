// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/Types.hpp"
// IWYU pragma: no_include "opentxs/opentxs.hpp"

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class Wallet;
}  // namespace internal
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Base : virtual public ::testing::Test
{
public:
    static auto CurrencyContract(
        const ot::api::Session& api,
        const ot::UnallocatedCString& nymid,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        const ot::UnitType unitOfAccount,
        const ot::Amount& redemptionIncrement,
        const ot::PasswordPrompt& reason) noexcept(false)
        -> ot::OTUnitDefinition;
    static auto InternalWallet(const ot::api::Session& api) noexcept
        -> const ot::api::session::internal::Wallet&;
    static auto NotaryContract(
        const ot::api::Session& api,
        const ot::identifier::Notary& id) noexcept -> ot::OTServerContract;
    static auto NotaryContract(
        const ot::api::Session& api,
        const ot::UnallocatedCString& nymid,
        const ot::UnallocatedCString& name,
        const ot::UnallocatedCString& terms,
        const ot::UnallocatedList<ot::contract::Server::Endpoint>& endpoints,
        const ot::PasswordPrompt& reason,
        const ot::VersionNumber version) noexcept -> ot::OTServerContract;
    static auto StopPair(const ot::api::session::Client& api) noexcept -> void;

protected:
    const ot::api::Context& ot_;

    Base() noexcept;

    ~Base() override = default;
};
}  // namespace ottest
