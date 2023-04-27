// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/dash/Dash.hpp"  // IWYU pragma: associated

namespace opentxs::crypto::implementation
{
auto Dash::Digest(const crypto::HashType, const ReadView, Writer&&)
    const noexcept -> bool
{
    return false;
}

auto Dash::HMAC(
    const crypto::HashType,
    const ReadView,
    const ReadView,
    Writer&&) const noexcept -> bool
{
    return false;
}
}  // namespace opentxs::crypto::implementation
