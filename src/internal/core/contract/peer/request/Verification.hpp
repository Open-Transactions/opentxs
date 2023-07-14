// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/Request.hpp"

namespace opentxs::contract::peer::request::internal
{
class Verification : virtual public peer::internal::Request
{
public:
    Verification(const Verification&) = delete;
    Verification(Verification&&) = delete;
    auto operator=(const Verification&) -> Verification& = delete;
    auto operator=(Verification&&) -> Verification& = delete;

    ~Verification() override = default;

protected:
    Verification() noexcept = default;
};
}  // namespace opentxs::contract::peer::request::internal
