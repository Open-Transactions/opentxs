// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <utility>

#include "internal/util/P0330.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/AccountActivity.hpp"
#include "ottest/fixtures/ui/AccountList.hpp"

namespace ottest
{
auto verify_account_balance(
    const int index,
    const ot::UnallocatedCString& account,
    const ot::Amount required) noexcept -> bool;
auto verify_response_codes(
    const ot::rpc::response::Base::Responses& codes,
    const std::size_t count,
    const ot::rpc::ResponseCode required =
        ot::rpc::ResponseCode::success) noexcept -> bool;

auto check_account_activity_rpc(
    const User& user,
    const ot::identifier::Account& account,
    const AccountActivityData& expected) noexcept -> bool
{
    auto output{true};
    const auto& api = *user.api_;
    const auto index{api.Instance()};
    const auto command = ot::rpc::request::GetAccountActivity{
        index, {account.asBase58(api.Crypto())}};
    const auto base = OTTestEnvironment::GetOT().RPC(command);
    const auto& response = base->asGetAccountActivity();
    const auto& codes = response.ResponseCodes();
    const auto& events = response.Activity();
    const auto vCount = expected.rows_.size();
    const auto goodCodes = verify_response_codes(
        codes,
        1,
        (0 == vCount) ? ot::rpc::ResponseCode::none
                      : ot::rpc::ResponseCode::success);
    output &= verify_account_balance(
        index, account.asBase58(api.Crypto()), expected.balance_);
    output &= goodCodes;
    output &= (events.size() == vCount);

    EXPECT_EQ(events.size(), vCount);

    auto v{expected.rows_.begin()};
    auto t{events.begin()};

    for (; (v != expected.rows_.end()) && (t != events.end()); ++v, ++t) {
        const auto& event = *t;
        const auto& required = *v;

        // TODO event.ContactID()
        // TODO event.State()
        // TODO event.Type()
        output &= (event.AccountID() == expected.id_);
        output &= (event.ConfirmedAmount() == required.amount_);
        output &= (event.Memo() == required.memo_);
        output &= (event.PendingAmount() == required.amount_);
        output &= (event.UUID() == required.uuid_);
        output &= (event.WorkflowID() == required.workflow_);

        EXPECT_EQ(event.AccountID(), expected.id_);
        EXPECT_EQ(event.ConfirmedAmount(), required.amount_);
        EXPECT_EQ(event.Memo(), required.memo_);
        EXPECT_EQ(event.PendingAmount(), required.amount_);
        EXPECT_EQ(event.UUID(), required.uuid_);
        EXPECT_EQ(event.WorkflowID(), required.workflow_);
    }

    return output;
}

auto check_account_list_rpc(
    const User& user,
    const AccountListData& expected) noexcept -> bool
{
    auto output{true};
    const auto& api = *user.api_;
    const auto index{api.Instance()};
    const auto command = ot::rpc::request::ListAccounts{
        index, user.nym_id_.asBase58(api.Crypto())};
    const auto base = OTTestEnvironment::GetOT().RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto& ids = response.AccountIDs();
    const auto goodCodes = verify_response_codes(codes, 1);
    const auto required = [&] {
        auto out = ot::UnallocatedVector<ot::UnallocatedCString>{};
        out.reserve(expected.rows_.size());

        for (const auto& row : expected.rows_) {
            out.emplace_back(row.account_id_);
        }

        std::sort(out.begin(), out.end());

        return out;
    }();
    const auto got = [&] {
        auto out{ids};
        std::sort(out.begin(), out.end());

        return out;
    }();
    output &= goodCodes;
    output &= (got == required);

    EXPECT_TRUE(goodCodes);
    EXPECT_EQ(got, required);

    return output;
}

auto verify_account_balance(
    const int index,
    const ot::UnallocatedCString& account,
    const ot::Amount required) noexcept -> bool
{
    auto output{true};
    const auto command = ot::rpc::request::GetAccountBalance{index, {account}};
    const auto base = OTTestEnvironment::GetOT().RPC(command);
    const auto& response = base->asGetAccountBalance();
    const auto& codes = response.ResponseCodes();
    const auto& data = response.Balances();
    const auto goodCodes = verify_response_codes(codes, 1);
    output &= goodCodes;
    output &= (data.size() == 1);

    EXPECT_TRUE(goodCodes);
    EXPECT_EQ(data.size(), 1);

    if (0 < data.size()) {
        const auto& item = data.front();

        // TODO item.ConfirmedBalance()
        // TODO item.ID()
        // TODO item.Issuer()
        // TODO item.Name()
        // TODO item.Owner()
        // TODO item.Type()
        // TODO item.Unit()

        output &= (item.PendingBalance() == required);

        EXPECT_EQ(item.PendingBalance(), required);
    }

    return output;
}

auto verify_response_codes(
    const ot::rpc::response::Base::Responses& codes,
    const std::size_t count,
    const ot::rpc::ResponseCode required) noexcept -> bool
{
    using namespace opentxs::literals;
    auto output{true};
    output &= (codes.size() == count);

    EXPECT_EQ(codes.size(), count);

    for (auto i = 0_uz; i < codes.size(); ++i) {
        const auto& code = codes.at(i);
        output &= (code.first == i);
        output &= (code.second == required);

        EXPECT_EQ(code.first, i);
        EXPECT_EQ(code.second, required);
    }

    return output;
}
}  // namespace ottest
