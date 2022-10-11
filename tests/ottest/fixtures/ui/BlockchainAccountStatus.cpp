// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/BlockchainAccountStatus.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <compare>
#include <iterator>
#include <sstream>
#include <string_view>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/BlockchainAccountStatus.hpp"
#include "internal/interface/ui/BlockchainSubaccount.hpp"
#include "internal/interface/ui/BlockchainSubaccountSource.hpp"
#include "internal/interface/ui/BlockchainSubchain.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace opentxs::literals;

auto check_blockchain_subaccounts(
    const ot::api::Session& api,
    const ot::ui::BlockchainSubaccountSource& widget,
    const ot::UnallocatedVector<BlockchainSubaccountData>& v) noexcept -> bool;
auto check_blockchain_subchains(
    const ot::ui::BlockchainSubaccount& widget,
    const ot::UnallocatedVector<BlockchainSubchainData>& v) noexcept -> bool;
}  // namespace ottest

namespace ottest
{
auto check_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool
{
    const auto& ot = *user.api_;
    const auto& widget =
        ot.UI().Internal().BlockchainAccountStatus(user.nym_id_, chain);
    auto output{true};
    output &= (widget.Chain() == expected.chain_);
    output &= (widget.Owner().asBase58(ot.Crypto()) == expected.owner_);

    EXPECT_EQ(widget.Chain(), expected.chain_);
    EXPECT_EQ(widget.Owner().asBase58(ot.Crypto()), expected.owner_);

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
        output &= (row->SourceID().asBase58(ot.Crypto()) == it->id_);
        output &= (row->Type() == it->type_);
        output &= check_blockchain_subaccounts(ot, row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->SourceID().asBase58(ot.Crypto()), it->id_);
        EXPECT_EQ(row->Type(), it->type_);

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

auto check_blockchain_subaccounts(
    const ot::api::Session& api,
    const ot::ui::BlockchainSubaccountSource& widget,
    const ot::UnallocatedVector<BlockchainSubaccountData>& v) noexcept -> bool
{
    auto output{true};
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
        output &= (row->SubaccountID().asBase58(api.Crypto()) == it->id_);
        output &= check_blockchain_subchains(row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->SubaccountID().asBase58(api.Crypto()), it->id_);

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

auto check_blockchain_subchains(
    const ot::ui::BlockchainSubaccount& widget,
    const ot::UnallocatedVector<BlockchainSubchainData>& v) noexcept -> bool
{
    auto output{true};
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
        output &= (row->Type() == it->type_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->Type(), it->type_);

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

auto init_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    Counter& counter) noexcept -> void
{
    user.api_->UI().Internal().BlockchainAccountStatus(
        user.nym_id_, chain, make_cb(counter, [&] {
            auto out = std::stringstream{};
            out << u8"blockchain_account_status_"_sv;
            out << user.name_lower_;

            return out.str();
        }()));
    wait_for_counter(counter);
}
}  // namespace ottest
