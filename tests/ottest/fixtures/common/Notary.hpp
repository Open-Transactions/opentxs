// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/Base.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Notary_fixture : virtual public Base,
                                      virtual public ::testing::Test
{
protected:
    using LocalNymMap =
        ot::UnallocatedMap<int, ot::UnallocatedSet<ot::UnallocatedCString>>;
    using SeedMap =
        ot::UnallocatedMap<int, ot::UnallocatedSet<ot::UnallocatedCString>>;

    static SeedMap seed_map_;
    static LocalNymMap local_nym_map_;

    using AccountMap = ot::UnallocatedMap<
        ot::UnallocatedCString,
        ot::UnallocatedVector<ot::UnallocatedCString>>;
    using IssuedUnits = ot::UnallocatedVector<ot::UnallocatedCString>;
    using UserIndex = ot::UnallocatedMap<int, User>;
    using AccountIndex = ot::UnallocatedMap<ot::UnitType, int>;

    static IssuedUnits created_units_;
    static AccountMap registered_accounts_;
    static AccountIndex account_index_;

    auto IssueUnit(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount,
        const ot::display::Definition& displayDefinition) const noexcept
        -> ot::UnallocatedCString;
    auto IssueUnit(
        const ot::api::session::Notary& server,
        const User& issuer,
        const ot::UnallocatedCString& shortname,
        const ot::UnallocatedCString& terms,
        ot::UnitType unitOfAccount,
        const ot::display::Definition& displayDefinition) const noexcept
        -> ot::UnallocatedCString;
    auto RefreshAccount(
        const ot::api::session::Client& api,
        const ot::identifier::Nym& nym,
        const ot::identifier::Notary& server) const noexcept -> void;
    auto RegisterNym(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::UnallocatedCString& nymID) const noexcept -> bool;
    auto RegisterNym(const ot::api::session::Notary& server, const User& nym)
        const noexcept -> bool;
    auto RegisterNym(
        const ot::api::session::Client& api,
        const ot::api::session::Notary& server,
        const ot::identifier::Nym& nym) const noexcept -> bool;
    auto StartNotarySession(int index) const noexcept
        -> const ot::api::session::Notary&;

    virtual auto CleanupNotary() noexcept -> void;

    Notary_fixture() noexcept = default;
};
}  // namespace ottest
