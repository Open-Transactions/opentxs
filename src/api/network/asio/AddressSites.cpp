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
            tls1_3,
            "ipv4.wtfismyip.com",
            "/text",
            ResponseType::AddressOnly,
            IPversion::IPV4,
        },
        {
            tls1_3,
            "ipv6.wtfismyip.com",
            "/text",
            ResponseType::AddressOnly,
            IPversion::IPV6,
        },
        {
            tls1_3,
            "api-ipv4.ip.sb",
            "/ip",
            ResponseType::AddressOnly,
            IPversion::IPV4,
        },
        {
            tls1_3,
            "api-ipv6.ip.sb",
            "/ip",
            ResponseType::AddressOnly,
            IPversion::IPV6,
        },
        {
            tls1_3,
            "api4.my-ip.io",
            "/ip",
            ResponseType::AddressOnly,
            IPversion::IPV4,
        },
        {
            tls1_3,
            "api6.my-ip.io",
            "/ip",
            ResponseType::AddressOnly,
            IPversion::IPV6,
        },
        {
            tls1_3,
            "ip4only.me",
            "/api/",
            ResponseType::IPvonly,
            IPversion::IPV4,
        },
        {
            tls1_3,
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
