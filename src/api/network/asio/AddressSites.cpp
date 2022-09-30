// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "api/network/asio/Shared.hpp"  // IWYU pragma: associated

namespace opentxs::api::network::asio
{
auto Shared::sites() -> const Vector<Site>&
{
    static const auto sites = Vector<Site>{
        {
            "ip4only.me",
            "http",
            "/api/",
            ResponseType::IPvonly,
            IPversion::IPV4,
            11,
        },
        {
            "ip6only.me",
            "http",
            "/api/",
            ResponseType::IPvonly,
            IPversion::IPV6,
            11,
        },
        {
            "ip4.seeip.org",
            "https",
            "/",
            ResponseType::AddressOnly,
            IPversion::IPV4,
            11,
        },
        {
            "ip6.seeip.org",
            "https",
            "/",
            ResponseType::AddressOnly,
            IPversion::IPV6,
            11,
        }};

    return sites;
}
}  // namespace opentxs::api::network::asio
