// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/library/Factory.hpp"  // IWYU pragma: associated

#include "crypto/library/dash/Dash.hpp"

namespace opentxs::factory
{
auto Dash() noexcept -> std::unique_ptr<crypto::Dash>
{
    using ReturnType = crypto::implementation::Dash;

    return std::make_unique<ReturnType>();
}
}  // namespace opentxs::factory
