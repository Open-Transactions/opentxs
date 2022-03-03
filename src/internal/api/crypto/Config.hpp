// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Config.hpp"

namespace opentxs::api::crypto::internal
{
class Config : virtual public api::crypto::Config
{
public:
    auto InternalConfig() const noexcept -> const Config& final
    {
        return *this;
    }

    auto InternalConfig() noexcept -> Config& final { return *this; }

    ~Config() override = default;
};
}  // namespace opentxs::api::crypto::internal
