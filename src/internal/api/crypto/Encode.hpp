// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Encode.hpp"

namespace opentxs::api::crypto::internal
{
class Encode : virtual public api::crypto::Encode
{
public:
    auto InternalEncode() const noexcept -> const Encode& final
    {
        return *this;
    }

    auto InternalEncode() noexcept -> Encode& final { return *this; }

    ~Encode() override = default;
};
}  // namespace opentxs::api::crypto::internal
