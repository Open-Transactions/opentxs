// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/api/network/OTDHT.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::internal
{
class OTDHT : public network::OTDHT
{
public:
    auto Internal() const noexcept -> const internal::OTDHT& final
    {
        return *this;
    }

    virtual auto Start(std::shared_ptr<const api::Session> api) noexcept
        -> void = 0;
    auto Internal() noexcept -> internal::OTDHT& final { return *this; }

    OTDHT(const OTDHT&) = delete;
    OTDHT(OTDHT&&) = delete;
    auto operator=(const OTDHT&) -> OTDHT& = delete;
    auto operator=(OTDHT&&) -> OTDHT& = delete;

    ~OTDHT() override = default;

protected:
    OTDHT() = default;
};
}  // namespace opentxs::api::network::internal
