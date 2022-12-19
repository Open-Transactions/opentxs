// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/Base.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Client_fixture : virtual public Base
{
protected:
    using UserIndex = ot::UnallocatedMap<int, User>;

    static UserIndex users_;

    auto CreateNym(
        const ot::api::session::Client& api,
        const ot::UnallocatedCString& name,
        const ot::UnallocatedCString& seed,
        int index) const noexcept -> const User&;
    auto ImportBip39(
        const ot::api::Session& api,
        const ot::UnallocatedCString& words) const noexcept
        -> ot::UnallocatedCString;
    auto ImportServerContract(
        const ot::api::session::Notary& from,
        const ot::api::session::Client& to) const noexcept -> bool;
    auto SetIntroductionServer(
        const ot::api::session::Client& on,
        const ot::api::session::Notary& to) const noexcept -> bool;
    auto StartClient(int index) const noexcept
        -> const ot::api::session::Client&;

    virtual auto CleanupClient() noexcept -> void;

    Client_fixture() noexcept = default;
};
}  // namespace ottest
