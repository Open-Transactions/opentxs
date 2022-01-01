// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/protobuf/verify/VerifyOTX.hpp"  // IWYU pragma: associated

#include <memory_resource>

namespace opentxs::proto
{
auto ServerReplyAllowedOTXPush() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
    };

    return output;
}
auto ServerReplyAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {3, 3}},
        {2, {3, 3}},
    };

    return output;
}
auto ServerRequestAllowedNym() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 5}},
        {2, {1, 6}},
    };

    return output;
}
auto ServerRequestAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {3, 3}},
        {2, {3, 3}},
    };

    return output;
}
}  // namespace opentxs::proto
