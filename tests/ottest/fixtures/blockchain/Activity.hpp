// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ottest
{
namespace ot = opentxs;

struct OPENTXS_EXPORT Test_BlockchainActivity : public ::testing::Test {
    using Element = ot::blockchain::crypto::Element;
    using Transaction = ot::blockchain::block::Transaction;

    static const ot::UnallocatedCString test_transaction_hex_;
    static const ot::UnallocatedCString btc_account_id_;
    static const ot::UnallocatedCString btc_unit_id_;
    static const ot::UnallocatedCString btc_notary_id_;
    static const ot::UnallocatedCString nym_1_name_;
    static const ot::UnallocatedCString nym_2_name_;
    static const ot::UnallocatedCString contact_3_name_;
    static const ot::UnallocatedCString contact_4_name_;
    static const ot::UnallocatedCString contact_5_name_;
    static const ot::UnallocatedCString contact_6_name_;
    static const ot::UnallocatedCString contact_7_name_;

    const ot::api::session::Client& api_;
    const ot::PasswordPrompt reason_;

    auto account_1_id() const noexcept -> const ot::identifier::Account&;
    auto account_2_id() const noexcept -> const ot::identifier::Account&;
    auto contact_1_id() const noexcept -> const ot::identifier::Generic&;
    auto contact_2_id() const noexcept -> const ot::identifier::Generic&;
    auto contact_3_id() const noexcept -> const ot::identifier::Generic&;
    auto contact_4_id() const noexcept -> const ot::identifier::Generic&;
    auto contact_5_id() const noexcept -> const ot::identifier::Generic&;
    auto contact_6_id() const noexcept -> const ot::identifier::Generic&;
    auto contact_7_id() const noexcept -> const ot::identifier::Generic&;
    auto get_test_transaction(
        const Element& first,
        const Element& second,
        const ot::Time& time = ot::Clock::now()) const -> Transaction;
    auto monkey_patch(const Element& first, const Element& second)
        const noexcept -> ot::UnallocatedCString;
    auto monkey_patch(
        const ot::UnallocatedCString& first,
        const ot::UnallocatedCString& second) const noexcept
        -> ot::UnallocatedCString;
    auto nym_1_id() const noexcept -> const ot::identifier::Nym&;
    auto nym_2_id() const noexcept -> const ot::identifier::Nym&;
    auto seed() const noexcept -> const ot::crypto::SeedID&;
    auto words() const noexcept -> const ot::UnallocatedCString&;

    Test_BlockchainActivity();
};
}  // namespace ottest
