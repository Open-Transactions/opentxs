// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Util.hpp"

namespace opentxs::api::crypto::internal
{
class Util : virtual public api::crypto::Util
{
public:
    auto InternalUtil() const noexcept -> const Util& final { return *this; }

    auto InternalUtil() noexcept -> Util& final { return *this; }

    ~Util() override = default;
};
}  // namespace opentxs::api::crypto::internal
