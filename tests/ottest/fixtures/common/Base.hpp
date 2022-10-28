// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/Export.hpp>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Context;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT Base : virtual public ::testing::Test
{
protected:
    const ot::api::Context& ot_;

    Base() noexcept;

    ~Base() override = default;
};
}  // namespace ottest
