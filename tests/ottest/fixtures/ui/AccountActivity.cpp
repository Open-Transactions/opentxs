// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/AccountActivity.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <iterator>
#include <sstream>
#include <string_view>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/BalanceItem.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

auto check_account_activity(
    const User& user,
    const ot::identifier::Account& account,
    const AccountActivityData& expected) noexcept -> bool
{
    const auto& widget =
        user.api_->UI().Internal().AccountActivity(user.nym_id_, account);
    auto output{true};
    output &= (widget.AccountID() == expected.id_);
    output &= (widget.Balance() == expected.balance_);
    output &= (widget.BalancePolarity() == expected.polarity_);
    output &= (widget.ContractID() == expected.contract_id_);
    output &= (widget.DepositChains() == expected.deposit_chains_);
    output &= (widget.DisplayBalance() == expected.display_balance_);
    output &= (widget.DisplayUnit() == expected.contract_name_);
    output &= (widget.Name() == expected.name_);
    output &= (widget.NotaryID() == expected.notary_id_);
    output &= (widget.NotaryName() == expected.notary_name_);
    output &=
        (std::to_string(widget.SyncPercentage()) ==
         std::to_string(expected.sync_));
    output &= (widget.SyncProgress() == expected.progress_);
    output &= (widget.Type() == expected.type_);
    output &= (widget.Unit() == expected.unit_);

    if (const auto& a = expected.default_deposit_address_; !a.empty()) {
        output &= (widget.DepositAddress() == a);

        EXPECT_EQ(widget.DepositAddress(), a);
    }

    for (const auto& [type, address] : expected.deposit_addresses_) {
        output &= (widget.DepositAddress(type) == address);

        EXPECT_EQ(widget.DepositAddress(type), address);
    }

    for (const auto& [input, valid] : expected.addresses_to_validate_) {
        const auto validated = widget.ValidateAddress(input);
        output &= (validated == valid);

        EXPECT_EQ(validated, valid);
    }

    for (const auto& [input, valid] : expected.amounts_to_validate_) {
        const auto validated = widget.ValidateAmount(input);
        output &= (validated == valid);

        EXPECT_EQ(validated, valid);
    }

    EXPECT_EQ(widget.AccountID(), expected.id_);
    EXPECT_EQ(widget.Balance(), expected.balance_);
    EXPECT_EQ(widget.BalancePolarity(), expected.polarity_);
    EXPECT_EQ(widget.ContractID(), expected.contract_id_);
    EXPECT_EQ(widget.DepositChains(), expected.deposit_chains_);
    EXPECT_EQ(widget.DisplayBalance(), expected.display_balance_);
    EXPECT_EQ(widget.DisplayUnit(), expected.contract_name_);
    EXPECT_EQ(widget.Name(), expected.name_);
    EXPECT_EQ(widget.NotaryID(), expected.notary_id_);
    EXPECT_EQ(widget.NotaryName(), expected.notary_name_);
    EXPECT_EQ(
        std::to_string(widget.SyncPercentage()),
        std::to_string(expected.sync_));
    EXPECT_EQ(widget.SyncProgress(), expected.progress_);
    EXPECT_EQ(widget.Type(), expected.type_);
    EXPECT_EQ(widget.Unit(), expected.unit_);

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
        output &= (row->Amount() == it->amount_);
        output &= (row->Confirmations() == it->confirmations_);
        output &= (row->Contacts() == it->contacts_);
        output &= (row->DisplayAmount() == it->display_amount_);
        output &= (row->Memo() == it->memo_);
        output &= (row->Workflow() == it->workflow_);
        output &= (row->Text() == it->text_);
        output &= (row->Type() == it->type_);
        output &= (row->UUID() == it->uuid_);

        if (const auto& time = it->timestamp_; time.has_value()) {
            output &= (row->Timestamp() == time.value());

            EXPECT_EQ(row->Timestamp(), time.value());
        }

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        EXPECT_EQ(row->Amount(), it->amount_);
        EXPECT_EQ(row->Confirmations(), it->confirmations_);
        EXPECT_EQ(row->Contacts(), it->contacts_);
        EXPECT_EQ(row->DisplayAmount(), it->display_amount_);
        EXPECT_EQ(row->Memo(), it->memo_);
        EXPECT_EQ(row->Workflow(), it->workflow_);
        EXPECT_EQ(row->Text(), it->text_);
        EXPECT_EQ(row->Type(), it->type_);
        EXPECT_EQ(row->UUID(), it->uuid_);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto init_account_activity(
    const User& user,
    const ot::identifier::Account& account,
    Counter& counter) noexcept -> void
{
    user.api_->UI().Internal().AccountActivity(
        user.nym_id_, account, make_cb(counter, [&] {
            auto out = std::stringstream{};
            out << u8"account_activity_"sv;
            out << user.name_lower_;

            return out.str();
        }()));
    wait_for_counter(counter);
}
}  // namespace ottest
