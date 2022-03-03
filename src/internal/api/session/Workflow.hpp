// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/Workflow.hpp"

namespace opentxs::api::session::internal
{
class Workflow : virtual public session::Workflow
{
public:
    auto Internal() const noexcept -> const internal::Workflow& final
    {
        return *this;
    }

    auto Internal() noexcept -> internal::Workflow& final { return *this; }

    ~Workflow() override = default;
};
}  // namespace opentxs::api::session::internal
