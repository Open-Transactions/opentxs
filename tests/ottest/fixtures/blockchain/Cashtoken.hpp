// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <string_view>

namespace ottest
{
namespace ot = opentxs;

struct OPENTXS_EXPORT Cashtoken : public ::testing::Test {
    const ot::api::session::Client& api_;

    auto ParseBlocks(std::string_view data) const noexcept -> bool;

    Cashtoken();

private:
    auto check_parsing_and_serialization(ot::ReadView raw) const noexcept
        -> bool;
};
}  // namespace ottest
