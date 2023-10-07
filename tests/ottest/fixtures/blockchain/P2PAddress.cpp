// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/P2PAddress.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ottest
{
P2PAddress::P2PAddress()
    : key_([&] {
        auto out = decltype(key_){};
        if (!ot_.Crypto().Encode().Z85Decode(key_z85_, out.WriteInto())) {
            ADD_FAILURE();
        }

        return out;
    }())
{
}
}  // namespace ottest

namespace ottest
{
const std::string P2PAddress::address_4_ = "54.39.129.45"s;
const std::string P2PAddress::address_6_ = "2607:5300:203:402d::"s;
}  // namespace ottest
