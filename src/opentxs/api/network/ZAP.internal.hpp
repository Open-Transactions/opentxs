// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/api/network/Types.hpp"

namespace opentxs::api::network::internal
{
class ZAP
{
public:
    virtual auto RegisterDomain(
        std::string_view domain,
        std::string_view handler) const noexcept -> bool = 0;
    virtual auto SetDefaultPolicy(ZAPPolicy policy) const noexcept -> bool = 0;

    virtual ~ZAP() = default;
};
}  // namespace opentxs::api::network::internal
