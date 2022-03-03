// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/VerifyConsensus.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::proto
{
auto ContextAllowedServer() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {2, 2}},
        {3, {3, 3}},
    };

    return output;
}
auto ContextAllowedClient() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
    };

    return output;
}
auto ContextAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {2, 2}},
        {2, {2, 2}},
        {3, {2, 2}},
    };

    return output;
}
auto ServerContextAllowedPendingCommand() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {3, {1, 1}},
    };

    return output;
}
auto ServerContextAllowedState() noexcept
    -> const UnallocatedMap<std::uint32_t, UnallocatedSet<int>>&
{
    static const auto output =
        UnallocatedMap<std::uint32_t, UnallocatedSet<int>>{
            {3, {1, 2, 3, 4, 5}}};

    return output;
}
auto ServerContextAllowedStatus() noexcept
    -> const UnallocatedMap<std::uint32_t, UnallocatedSet<int>>&
{
    static const auto output =
        UnallocatedMap<std::uint32_t, UnallocatedSet<int>>{
            {3, {1, 2, 3, 4, 5}}};

    return output;
}
}  // namespace opentxs::proto
