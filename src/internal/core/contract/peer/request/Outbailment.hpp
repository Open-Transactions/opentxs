// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/peer/Request.hpp"

namespace opentxs::contract::peer::request::internal
{
class Outbailment : virtual public peer::internal::Request
{
public:
    Outbailment(const Outbailment&) = delete;
    Outbailment(Outbailment&&) = delete;
    auto operator=(const Outbailment&) -> Outbailment& = delete;
    auto operator=(Outbailment&&) -> Outbailment& = delete;

    ~Outbailment() override = default;

protected:
    Outbailment() noexcept = default;
};
}  // namespace opentxs::contract::peer::request::internal
