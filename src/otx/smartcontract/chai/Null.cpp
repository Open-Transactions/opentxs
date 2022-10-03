// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "internal/otx/smartcontract/Factory.hpp"  // IWYU pragma: associated

#include "internal/otx/smartcontract/OTScript.hpp"

namespace opentxs::factory
{
auto OTScriptChai() -> std::shared_ptr<opentxs::OTScript>
{
    return std::make_shared<opentxs::OTScript>();
}

auto OTScriptChai(const UnallocatedCString&)
    -> std::shared_ptr<opentxs::OTScript>
{
    return std::make_shared<opentxs::OTScript>();
}
}  // namespace opentxs::factory
