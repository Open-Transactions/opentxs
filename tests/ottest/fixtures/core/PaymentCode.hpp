// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

namespace ottest
{
namespace ot = opentxs;

using namespace std::literals;

class OPENTXS_EXPORT PaymentCode : public ::testing::Test
{
public:
    static const bool have_hd_;

    const ot::api::session::Client& api_;
    ot::PasswordPrompt reason_;
    ot::UnallocatedCString seed_, fingerprint_, nym_id_0_, paycode_0_,
        nym_id_1_, paycode_1_, nym_id_2_, paycode_2_, nym_id_3_, paycode_3_;
    ot::NymData nym_data_0_, nym_data_1_, nym_data_2_, nym_data_3_;
    ot::UnitType currency_ = ot::UnitType::Bch;
    ot::UnitType currency_2_ = ot::UnitType::Btc;

    PaymentCode();
};
}  // namespace ottest

