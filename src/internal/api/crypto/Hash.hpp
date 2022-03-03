// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/crypto/Hash.hpp"

namespace opentxs::api::crypto::internal
{
class Hash : virtual public api::crypto::Hash
{
public:
    auto InternalHash() const noexcept -> const Hash& final { return *this; }

    auto InternalHash() noexcept -> Hash& final { return *this; }

    ~Hash() override = default;
};
}  // namespace opentxs::api::crypto::internal
