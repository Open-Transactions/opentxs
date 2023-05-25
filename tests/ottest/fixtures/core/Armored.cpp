// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/Armored.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/api/FactoryAPI.hpp"

namespace ottest
{
Armored::Armored() noexcept = default;

auto Armored::Check(const ot::Data& data) noexcept(false) -> bool
{
    const auto armored = ot_.Factory().Internal().Armored(data);
    const auto recovered = ot_.Factory().Data(armored);

    EXPECT_EQ(recovered, data);

    return recovered == data;
}
}  // namespace ottest
