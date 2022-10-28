// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/Export.hpp>

namespace ottest
{
class OPENTXS_EXPORT OTTestEnvironment : public testing::Environment
{
public:
    void SetUp() override;
    void TearDown() override;

    OTTestEnvironment() noexcept;

    ~OTTestEnvironment() override;
};
}  // namespace ottest
