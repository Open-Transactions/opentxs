// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/NymList.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <compare>
#include <iterator>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/NymList.hpp"
#include "internal/interface/ui/NymListItem.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"

namespace ottest
{
Counter NymList::counter_{};
NymListData NymList::expected_{};

NymList::NymList()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
{
}

auto check_nym_list(
    const ot::api::session::Client& api,
    const NymListData& expected) noexcept -> bool
{
    const auto& widget = api.UI().Internal().NymList();
    auto output{true};
    const auto& v = expected.rows_;
    auto row = widget.First();

    if (const auto valid = row->Valid(); 0 < v.size()) {
        output &= valid;

        EXPECT_TRUE(valid);

        if (false == valid) { return output; }
    } else {
        output &= (false == valid);

        EXPECT_FALSE(valid);
    }

    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        output &= (row->Name() == it->name_);
        output &= (row->NymID() == it->id_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->NymID(), it->id_);

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto init_nym_list(
    const ot::api::session::Client& api,
    Counter& counter) noexcept -> void
{
    api.UI().Internal().NymList(make_cb(counter, "nym_list"));
}
}  // namespace ottest
