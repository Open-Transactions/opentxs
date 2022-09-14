// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string_view>

#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
namespace internal
{
class OTDHT;
}  // namespace internal
}  // namespace network
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
class OPENTXS_EXPORT OTDHT
{
public:
    using Endpoints = Vector<CString>;

    virtual auto AddPeer(std::string_view endpoint) const noexcept -> bool = 0;
    virtual auto DeletePeer(std::string_view endpoint) const noexcept
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::OTDHT& = 0;
    virtual auto KnownPeers(alloc::Default alloc) const noexcept
        -> Endpoints = 0;
    virtual auto StartListener(
        std::string_view syncEndpoint,
        std::string_view publicSyncEndpoint,
        std::string_view updateEndpoint,
        std::string_view publicUpdateEndpoint) const noexcept -> bool = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::OTDHT& = 0;

    OTDHT(const OTDHT&) = delete;
    OTDHT(OTDHT&&) = delete;
    auto operator=(const OTDHT&) -> OTDHT& = delete;
    auto operator=(OTDHT&&) -> OTDHT& = delete;

    OPENTXS_NO_EXPORT virtual ~OTDHT() = default;

protected:
    OTDHT() = default;
};
}  // namespace opentxs::api::network
