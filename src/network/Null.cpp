// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                 // IWYU pragma: associated
#include "1_Internal.hpp"               // IWYU pragma: associated
#include "opentxs/network/OpenDHT.hpp"  // IWYU pragma: associated

#include "internal/network/Factory.hpp"

namespace opentxs::network::implementation
{
struct OpenDHT final : public network::OpenDHT {
    auto Insert(
        const UnallocatedCString&,
        const UnallocatedCString&,
        DhtDoneCallback) const noexcept -> void final
    {
    }
    auto Retrieve(
        const UnallocatedCString&,
        DhtResultsCallback,
        DhtDoneCallback) const noexcept -> void final
    {
    }

    ~OpenDHT() final = default;
};
}  // namespace opentxs::network::implementation

namespace opentxs::factory
{
auto OpenDHT(const network::DhtConfig&) noexcept
    -> std::unique_ptr<network::OpenDHT>
{
    using ReturnType = network::implementation::OpenDHT;

    return std::make_unique<ReturnType>();
}
}  // namespace opentxs::factory
