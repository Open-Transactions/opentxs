// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/ActivityThread.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <iterator>
#include <sstream>
#include <string_view>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/ActivityThread.hpp"
#include "internal/interface/ui/ActivityThreadItem.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace opentxs::literals;

auto activity_thread_request_faucet(
    const User& user,
    const User& remote) noexcept -> bool
{
    const auto& widget = user.api_->UI().Internal().ActivityThread(
        user.nym_id_, user.Contact(remote.name_));

    return widget.SendFaucetRequest(ot::UnitType::Regtest);
}

auto activity_thread_send_message(const User& user, const User& remote) noexcept
    -> bool
{
    const auto& widget = user.api_->UI().Internal().ActivityThread(
        user.nym_id_, user.Contact(remote.name_));

    return widget.SendDraft();
}

auto activity_thread_send_message(
    const User& user,
    const User& remote,
    const ot::UnallocatedCString& messasge) noexcept -> bool
{
    const auto& widget = user.api_->UI().Internal().ActivityThread(
        user.nym_id_, user.Contact(remote.name_));
    const auto set = widget.SetDraft(messasge);
    const auto sent = widget.SendDraft();

    EXPECT_TRUE(set);
    EXPECT_TRUE(sent);

    return set && sent;
}

auto check_activity_thread(
    const User& user,
    const ot::identifier::Generic& contact,
    const ActivityThreadData& expected) noexcept -> bool
{
    const auto& widget =
        user.api_->UI().Internal().ActivityThread(user.nym_id_, contact);
    auto output{true};
    output &= (widget.CanMessage() == expected.can_message_);
    output &= (widget.DisplayName() == expected.display_name_);
    output &= (widget.GetDraft() == expected.draft_);
    output &= (widget.Participants() == expected.participants_);
    output &= (widget.ThreadID() == expected.thread_id_);

    EXPECT_EQ(widget.CanMessage(), expected.can_message_);
    EXPECT_EQ(widget.DisplayName(), expected.display_name_);
    EXPECT_EQ(widget.GetDraft(), expected.draft_);
    EXPECT_EQ(widget.Participants(), expected.participants_);
    EXPECT_EQ(widget.ThreadID(), expected.thread_id_);

    for (const auto& [type, required] : expected.payment_codes_) {
        const auto existing = widget.PaymentCode(type);

        output &= (existing == required);

        EXPECT_EQ(existing, required);
    }

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
        output &= (row->Loading() == it->loading_);
        output &= (row->Pending() == it->pending_);
        output &= (row->Amount() == it->amount_);
        output &= (row->DisplayAmount() == it->display_amount_);
        output &= (row->From() == it->from_);
        output &= (row->Memo() == it->memo_);
        output &= (row->Outgoing() == it->outgoing_);
        output &= (row->Text() == it->text_);
        output &= (row->Type() == it->type_);
        output &= (row->TXID() == it->txid_);

        EXPECT_EQ(row->Loading(), it->loading_);
        EXPECT_EQ(row->Pending(), it->pending_);
        EXPECT_EQ(row->Amount(), it->amount_);
        EXPECT_EQ(row->DisplayAmount(), it->display_amount_);
        EXPECT_EQ(row->From(), it->from_);
        EXPECT_EQ(row->Memo(), it->memo_);
        EXPECT_EQ(row->Outgoing(), it->outgoing_);
        EXPECT_EQ(row->Text(), it->text_);
        EXPECT_EQ(row->Type(), it->type_);
        EXPECT_EQ(row->TXID(), it->txid_);

        if (it->timestamp_.has_value()) {
            const auto& time = it->timestamp_.value();

            output &= (row->Timestamp() == time);

            EXPECT_EQ(row->Timestamp(), time);
        } else {
            static const auto null = ot::Time{};

            output &= (row->Timestamp() != null);

            EXPECT_NE(row->Timestamp(), null);
        }

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

auto init_activity_thread(
    const User& user,
    const User& remote,
    Counter& counter) noexcept -> void
{
    user.api_->UI().Internal().ActivityThread(
        user.nym_id_, user.Contact(remote.name_), make_cb(counter, [&] {
            auto out = std::stringstream{};
            out << u8"activity_thread_"_sv;
            out << user.name_lower_;
            out << '_';
            out << remote.name_lower_;

            return out.str();
        }()));
    wait_for_counter(counter);
}
}  // namespace ottest
