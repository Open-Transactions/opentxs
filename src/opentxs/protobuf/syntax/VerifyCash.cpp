// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/VerifyCash.hpp"  // IWYU pragma: associated

namespace opentxs::protobuf::inline syntax
{
auto LucreTokenDataAllowedCiphertext() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto PurseAllowedCiphertext() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto PurseAllowedEnvelope() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {2, 2}},
    };

    return output;
}
auto PurseAllowedSymmetricKey() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 2}},
    };

    return output;
}
auto PurseAllowedToken() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto PurseExchangeAllowedPurse() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto TokenAllowedLucreTokenData() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
}  // namespace opentxs::protobuf::inline syntax
