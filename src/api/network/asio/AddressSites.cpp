// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/asio/Shared.hpp"  // IWYU pragma: associated

namespace opentxs::api::network::asio
{
auto Shared::sites() -> const Vector<Site>&
{
    using enum opentxs::network::asio::TLS;
    static const auto sites = Vector<Site>{
        {
            std::nullopt,
            "ip4only.me",
            "/api/",
            ResponseType::IPvonly,
            IPversion::IPV4,
        },
        {
            std::nullopt,
            "ip6only.me",
            "/api/",
            ResponseType::IPvonly,
            IPversion::IPV6,
        },
        {
            tls1_2,
            "ip4.seeip.org",
            "/",
            ResponseType::AddressOnly,
            IPversion::IPV4,
        },
        {
            tls1_2,
            "ip6.seeip.org",
            "/",
            ResponseType::AddressOnly,
            IPversion::IPV6,
        }};

    return sites;
}
}  // namespace opentxs::api::network::asio
