// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <iterator>
#include <sstream>

#include "integration/Helpers.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/ui/BlockchainAccountStatus.hpp"
#include "opentxs/ui/BlockchainSelection.hpp"
#include "opentxs/ui/BlockchainSelectionItem.hpp"
#include "opentxs/ui/BlockchainSubaccount.hpp"
#include "opentxs/ui/BlockchainSubaccountSource.hpp"
#include "opentxs/ui/BlockchainSubchain.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace ottest
{
auto activity_thread_send_message(const User& user, const User& remote) noexcept
    -> bool
{
    const auto& widget = user.api_->UI().ActivityThread(
        user.nym_id_, user.Contact(remote.name_));

    return widget.SendDraft();
}

auto activity_thread_send_message(
    const User& user,
    const User& remote,
    const std::string& messasge) noexcept -> bool
{
    const auto& widget = user.api_->UI().ActivityThread(
        user.nym_id_, user.Contact(remote.name_));
    const auto set = widget.SetDraft(messasge);
    const auto sent = widget.SendDraft();

    EXPECT_TRUE(set);
    EXPECT_TRUE(sent);

    return set && sent;
}

auto check_blockchain_subaccounts(
    const ot::ui::BlockchainSubaccountSource& widget,
    const std::pmr::vector<BlockchainSubaccountData>& v) noexcept -> bool;
auto check_blockchain_subchains(
    const ot::ui::BlockchainSubaccount& widget,
    const std::pmr::vector<BlockchainSubchainData>& v) noexcept -> bool;

auto check_account_activity(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool
{
    const auto& widget = user.api_->UI().AccountActivity(user.nym_id_, account);
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

        if (auto& time = it->timestamp_; time.has_value()) {
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

auto check_account_list(
    const User& user,
    const AccountListData& expected) noexcept -> bool
{
    const auto& widget = user.api_->UI().AccountList(user.nym_id_);
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
        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (row->AccountID() == it->account_id_);
        output &= (row->Balance() == it->balance_);
        output &= (row->ContractID() == it->contract_id_);
        output &= (row->DisplayBalance() == it->display_balance_);
        output &= (row->DisplayUnit() == it->display_unit_);
        output &= (row->Name() == it->name_);
        output &= (row->NotaryID() == it->notary_id_);
        output &= (row->NotaryName() == it->notary_name_);
        output &= (row->Type() == it->type_);
        output &= (row->Unit() == it->unit_);
        output &= (lastVector == lastRow);

        EXPECT_EQ(row->AccountID(), it->account_id_);
        EXPECT_EQ(row->Balance(), it->balance_);
        EXPECT_EQ(row->ContractID(), it->contract_id_);
        EXPECT_EQ(row->DisplayBalance(), it->display_balance_);
        EXPECT_EQ(row->DisplayUnit(), it->display_unit_);
        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->NotaryID(), it->notary_id_);
        EXPECT_EQ(row->NotaryName(), it->notary_name_);
        EXPECT_EQ(row->Type(), it->type_);
        EXPECT_EQ(row->Unit(), it->unit_);
        EXPECT_EQ(lastVector, lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto check_activity_thread(
    const User& user,
    const ot::Identifier& contact,
    const ActivityThreadData& expected) noexcept -> bool
{
    const auto& widget = user.api_->UI().ActivityThread(user.nym_id_, contact);
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

        EXPECT_EQ(row->Loading(), it->loading_);
        EXPECT_EQ(row->Pending(), it->pending_);
        EXPECT_EQ(row->Amount(), it->amount_);
        EXPECT_EQ(row->DisplayAmount(), it->display_amount_);
        EXPECT_EQ(row->From(), it->from_);
        EXPECT_EQ(row->Memo(), it->memo_);
        EXPECT_EQ(row->Outgoing(), it->outgoing_);
        EXPECT_EQ(row->Text(), it->text_);
        EXPECT_EQ(row->Type(), it->type_);

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

auto check_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool
{
    const auto& widget =
        user.api_->UI().BlockchainAccountStatus(user.nym_id_, chain);
    auto output{true};
    output &= (widget.Chain() == expected.chain_);
    output &= (widget.Owner().str() == expected.owner_);

    EXPECT_EQ(widget.Chain(), expected.chain_);
    EXPECT_EQ(widget.Owner().str(), expected.owner_);

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
        output &= (row->SourceID().str() == it->id_);
        output &= (row->Type() == it->type_);
        output &= check_blockchain_subaccounts(row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->SourceID().str(), it->id_);
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

auto check_blockchain_selection(
    const ot::api::session::Client& api,
    const ot::ui::Blockchains type,
    const BlockchainSelectionData& expected) noexcept -> bool
{
    const auto& widget = api.UI().BlockchainSelection(type);
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
        output &= (row->IsEnabled() == it->enabled_);
        output &= (row->IsTestnet() == it->testnet_);
        output &= (row->Type() == it->type_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->IsEnabled(), it->enabled_);
        EXPECT_EQ(row->IsTestnet(), it->testnet_);
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
    const ot::ui::BlockchainSubaccountSource& widget,
    const std::pmr::vector<BlockchainSubaccountData>& v) noexcept -> bool
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
        output &= (row->SubaccountID().str() == it->id_);
        output &= check_blockchain_subchains(row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->SubaccountID().str(), it->id_);

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
    const std::pmr::vector<BlockchainSubchainData>& v) noexcept -> bool
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

auto check_contact_list(
    const User& user,
    const ContactListData& expected) noexcept -> bool
{
    const auto& widget = user.api_->UI().ContactList(user.nym_id_);
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
        const auto& index = it->contact_id_index_;

        if (it->check_contact_id_) {
            const auto match = (row->ContactID() == user.Contact(index).str());

            output &= match;

            EXPECT_EQ(row->ContactID(), user.Contact(index).str());
        } else {
            const auto set = user.SetContact(index, row->ContactID());
            const auto exists = (false == user.Contact(index).empty());

            output &= set;
            output &= exists;

            EXPECT_TRUE(set);
            EXPECT_TRUE(exists);
        }

        output &= (row->DisplayName() == it->name_);
        output &= (row->ImageURI() == it->image_);
        output &= (row->Section() == it->section_);

        EXPECT_EQ(row->DisplayName(), it->name_);
        EXPECT_EQ(row->ImageURI(), it->image_);
        EXPECT_EQ(row->Section(), it->section_);

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

auto check_messagable_list(
    const User& user,
    const ContactListData& expected) noexcept -> bool
{
    const auto& widget = user.api_->UI().MessagableList(user.nym_id_);
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
        const auto& index = it->contact_id_index_;

        if (it->check_contact_id_) {
            const auto match = (row->ContactID() == user.Contact(index).str());

            output &= match;

            EXPECT_EQ(row->ContactID(), user.Contact(index).str());
        } else {
            const auto set = user.SetContact(index, row->ContactID());
            const auto exists = (false == user.Contact(index).empty());

            output &= set;
            output &= exists;

            EXPECT_TRUE(set);
            EXPECT_TRUE(exists);
        }

        output &= (row->DisplayName() == it->name_);
        output &= (row->ImageURI() == it->image_);
        output &= (row->Section() == it->section_);

        EXPECT_EQ(row->DisplayName(), it->name_);
        EXPECT_EQ(row->ImageURI(), it->image_);
        EXPECT_EQ(row->Section(), it->section_);

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

auto contact_list_add_contact(
    const User& user,
    const std::string& label,
    const std::string& paymentCode,
    const std::string& nymID) noexcept -> std::string
{
    const auto& widget = user.api_->UI().ContactList(user.nym_id_);

    return widget.AddContact(label, paymentCode, nymID);
}

auto make_cb(Counter& counter, const std::string name) noexcept
    -> std::function<void()>
{
    return [&counter, name]() {
        auto& [expected, value] = counter;

        if (++value > expected) { std::cout << name << ": " << value << '\n'; }
    };
}

auto init_account_activity(
    const User& user,
    const ot::Identifier& account,
    Counter& counter) noexcept -> void
{
    user.api_->UI().AccountActivity(
        user.nym_id_, account, make_cb(counter, [&] {
            auto out = std::stringstream{};
            out << u8"account_activity_";
            out << user.name_lower_;

            return out.str();
        }()));
    wait_for_counter(counter);
}

auto init_account_list(const User& user, Counter& counter) noexcept -> void
{
    user.api_->UI().AccountList(user.nym_id_, make_cb(counter, [&] {
                                    auto out = std::stringstream{};
                                    out << u8"account_list_";
                                    out << user.name_lower_;

                                    return out.str();
                                }()));
    wait_for_counter(counter);
}

auto init_activity_thread(
    const User& user,
    const User& remote,
    Counter& counter) noexcept -> void
{
    user.api_->UI().ActivityThread(
        user.nym_id_, user.Contact(remote.name_), make_cb(counter, [&] {
            auto out = std::stringstream{};
            out << u8"activity_thread_";
            out << user.name_lower_;
            out << '_';
            out << remote.name_lower_;

            return out.str();
        }()));
    wait_for_counter(counter);
}

auto init_contact_list(const User& user, Counter& counter) noexcept -> void
{
    user.api_->UI().ContactList(user.nym_id_, make_cb(counter, [&] {
                                    auto out = std::stringstream{};
                                    out << u8"contact_list_";
                                    out << user.name_lower_;

                                    return out.str();
                                }()));
    wait_for_counter(counter);
}

auto init_messagable_list(const User& user, Counter& counter) noexcept -> void
{
    user.api_->UI().MessagableList(user.nym_id_, make_cb(counter, [&] {
                                       auto out = std::stringstream{};
                                       out << u8"messagable_list_";
                                       out << user.name_lower_;

                                       return out.str();
                                   }()));
    wait_for_counter(counter);
}

auto wait_for_counter(Counter& data, const bool hard) noexcept -> bool
{
    const auto limit =
        hard ? std::chrono::seconds(300) : std::chrono::seconds(30);
    auto start = ot::Clock::now();
    auto& [expected, updated] = data;

    while ((updated < expected) && ((ot::Clock::now() - start) < limit)) {
        ot::Sleep(std::chrono::milliseconds(100));
    }

    if (false == hard) { updated.store(expected.load()); }

    return updated >= expected;
}
}  // namespace ottest
