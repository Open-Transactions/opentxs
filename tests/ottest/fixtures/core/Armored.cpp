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
    auto out{true};

    {
        const auto& factory = ot_.Factory();
        const auto armored = factory.Internal().Armored(data);
        const auto recovered = factory.Data(armored);

        EXPECT_EQ(recovered, data);

        out &= (recovered == data);
    }
    {
        const auto& crypto = ot_.Crypto().Encode();
        auto encoded = opentxs::UnallocatedCString{};
        auto recovered = opentxs::ByteArray{};
        const auto armored =
            crypto.Armor(data.Bytes(), opentxs::writer(encoded), "DATA");
        const auto dearmored = crypto.Dearmor(encoded, recovered.WriteInto());

        EXPECT_TRUE(armored);
        EXPECT_TRUE(dearmored);
        EXPECT_EQ(data, recovered);

        out &= (armored && dearmored && (data == recovered));
    }

    return out;
}
}  // namespace ottest
