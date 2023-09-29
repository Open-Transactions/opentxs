// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
inline namespace literals
{
constexpr auto operator""_kib(const unsigned long long int in)
    -> unsigned long long int
{
    return in * 1024u;
}

constexpr auto operator""_mib(const unsigned long long int in)
    -> unsigned long long int
{
    return in * 1024u * 1024u;
}

constexpr auto operator""_gib(const unsigned long long int in)
    -> unsigned long long int
{
    return in * 1024u * 1024u * 1024u;
}

constexpr auto operator""_tib(const unsigned long long int in)
    -> unsigned long long int
{
    static_assert(8u <= sizeof(unsigned long long int));

    return in * 1024u * 1024u * 1024u * 1024u;
}
}  // namespace literals
}  // namespace opentxs
