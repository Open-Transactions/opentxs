// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/Reply.hpp"

namespace opentxs::contract::peer::reply::internal
{
class Connection : virtual public peer::internal::Reply
{
public:
    Connection(const Connection&) = delete;
    Connection(Connection&&) = delete;
    auto operator=(const Connection&) -> Connection& = delete;
    auto operator=(Connection&&) -> Connection& = delete;

    ~Connection() override = default;

protected:
    Connection() noexcept = default;
};
}  // namespace opentxs::contract::peer::reply::internal
