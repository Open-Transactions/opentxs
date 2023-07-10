// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/Reply.hpp"

namespace opentxs::contract::peer::reply::internal
{
class StoreSecret : virtual public peer::internal::Reply
{
public:
    StoreSecret(const StoreSecret&) = delete;
    StoreSecret(StoreSecret&&) = delete;
    auto operator=(const StoreSecret&) -> StoreSecret& = delete;
    auto operator=(StoreSecret&&) -> StoreSecret& = delete;

    ~StoreSecret() override = default;

protected:
    StoreSecret() noexcept = default;
};
}  // namespace opentxs::contract::peer::reply::internal
