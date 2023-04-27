// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/HashingProvider.hpp"

namespace opentxs::crypto
{
class Dash : virtual public HashingProvider
{
public:
    Dash(const Dash&) = delete;
    Dash(Dash&&) = delete;
    auto operator=(const Dash&) -> Dash& = delete;
    auto operator=(Dash&&) -> Dash& = delete;

    ~Dash() override = default;

protected:
    Dash() = default;
};
}  // namespace opentxs::crypto
