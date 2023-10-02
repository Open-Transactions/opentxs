// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/ui/BlockchainAccountStatus.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <compare>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string_view>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/BlockchainAccountStatus.hpp"
#include "internal/interface/ui/BlockchainSubaccount.hpp"
#include "internal/interface/ui/BlockchainSubaccountSource.hpp"
#include "internal/interface/ui/BlockchainSubchain.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

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
        output &= (row->SourceID() == it->id_);
        output &= (row->Type() == it->type_);
        output &= check_blockchain_subaccounts(ot, row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->SourceID(), it->id_);
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
            out << u8"blockchain_account_status_"sv;
            out << user.name_lower_;

            return out.str();
        }()));
    wait_for_counter(counter);
}

std::optional<User> BlockchainAccountStatus::alice_s_{std::nullopt};
std::optional<User> BlockchainAccountStatus::bob_s_{std::nullopt};
std::optional<User> BlockchainAccountStatus::chris_s_{std::nullopt};
BlockchainAccountStatus::HDAccountMap BlockchainAccountStatus::hd_acct_{};
BlockchainAccountStatus::PCAccountMap BlockchainAccountStatus::pc_acct_{};

auto BlockchainAccountStatus::Account(
    const User& user,
    ot::blockchain::Type chain) const noexcept
    -> const ot::blockchain::crypto::Account&
{
    return user.api_->Crypto().Blockchain().Account(user.nym_id_, chain);
}

auto BlockchainAccountStatus::make_hd_account(
    const User& user,
    const Protocol type) noexcept -> void
{
    hd_acct_[user.nym_id_].emplace(
        type,
        user.api_->Crypto().Blockchain().NewHDSubaccount(
            user.nym_id_, type, chain_, user.Reason()));
}
auto BlockchainAccountStatus::make_pc_account(const User& local,
    const User& remote) noexcept -> void
{
    const auto& api = *local.api_;
    const auto path = [&] {
        auto out = api.Factory().Data();
        local.nym_->PaymentCodePath(out.WriteInto());

        return out;
    }();
    pc_acct_[local.payment_code_].emplace(
        remote.payment_code_,
        api.Crypto()
            .Blockchain()
            .LoadOrCreateSubaccount(
                local.nym_id_,
                api.Factory().PaymentCodeFromBase58(remote.payment_code_),
                chain_,
                local.Reason())
            .ID());
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-reference"  // NOLINT
BlockchainAccountStatus::BlockchainAccountStatus()
    : alice_([&]() -> auto& {
        if (false == alice_s_.has_value()) {
            const auto& v = GetPaymentCodeVector3().alice_;
            alice_s_.emplace(v.words_, "Alice");
            alice_s_->init(OTTestEnvironment::GetOT().StartClientSession(0));
        }

        return alice_s_.value();
    }())
    , bob_([&]() -> auto& {
        if (false == bob_s_.has_value()) {
            const auto& v = GetPaymentCodeVector3().bob_;
            bob_s_.emplace(v.words_, "Bob");
            bob_s_->init(OTTestEnvironment::GetOT().StartClientSession(1));
        }

        return bob_s_.value();
    }())
    , chris_([&]() -> auto& {
        if (false == chris_s_.has_value()) {
            chris_s_.emplace(pkt_words_, "Chris", pkt_passphrase_);
            chris_s_->init(
                OTTestEnvironment::GetOT().StartClientSession(1),
                ot::identity::Type::individual,
                0,
                ot::crypto::SeedStyle::PKT);
        }

        return chris_s_.value();
    }())
{
}
#pragma GCC diagnostic pop
}  // namespace ottest
