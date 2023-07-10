// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/Reply.hpp"

namespace opentxs::contract::peer::reply::internal
{
class Bailment : virtual public peer::internal::Reply
{
public:
    Bailment(const Bailment&) = delete;
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;

    ~Bailment() override = default;

protected:
    Bailment() noexcept = default;
};
}  // namespace opentxs::contract::peer::reply::internal
