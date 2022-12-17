// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/common/PasswordCallback.hpp"

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT LowLevel : virtual public ::testing::Test
{
private:
    ot::PasswordCaller caller_;

protected:
    PasswordCallback password_;
    const ot::api::Context& ot_;

    LowLevel() noexcept;
    LowLevel(const ot::Options& args) noexcept;

    ~LowLevel() override;
};
}  // namespace ottest
