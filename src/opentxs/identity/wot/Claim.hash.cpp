// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/Claim.hpp"  // IWYU pragma: associated

#include "opentxs/core/identifier/Generic.hpp"

auto std::hash<opentxs::identity::wot::Claim>::operator()(
    const opentxs::identity::wot::Claim& rhs) const noexcept -> std::size_t
{
    static const auto hasher =
        hash<opentxs::identity::wot::Claim::identifier_type>{};

    return hasher(rhs.ID());
}
