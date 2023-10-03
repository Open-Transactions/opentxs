// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <functional>
#include <optional>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT BlockchainSubchainData {
    ot::UnallocatedCString name_;
    ot::blockchain::crypto::Subchain type_;
};

struct OPENTXS_EXPORT BlockchainSubaccountData {
    ot::UnallocatedCString name_;
    ot::UnallocatedCString id_;
    ot::UnallocatedVector<BlockchainSubchainData> rows_;
};

struct OPENTXS_EXPORT BlockchainSubaccountSourceData {
    ot::UnallocatedCString name_;
    ot::identifier::Generic id_;
    ot::blockchain::crypto::SubaccountType type_;
    ot::UnallocatedVector<BlockchainSubaccountData> rows_;
};

struct OPENTXS_EXPORT BlockchainAccountStatusData {
    ot::UnallocatedCString owner_;
    ot::blockchain::Type chain_;
    ot::UnallocatedVector<BlockchainSubaccountSourceData> rows_;
};

OPENTXS_EXPORT auto check_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_blockchain_account_status_qt(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;
OPENTXS_EXPORT auto init_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    Counter& counter) noexcept -> void;

class OPENTXS_EXPORT BlockchainAccountStatus : public ::testing::Test
{
public:
    using Protocol = ot::blockchain::crypto::HDProtocol;
    using Subaccount = ot::blockchain::crypto::SubaccountType;
    using Subchain = ot::blockchain::crypto::Subchain;
    using HDAccountMap = ot::UnallocatedMap<
        ot::identifier::Nym,
        ot::UnallocatedMap<Protocol, ot::identifier::Generic>>;
    using PCAccountMap = std::map<
        ot::UnallocatedCString,
        ot::UnallocatedMap<ot::UnallocatedCString, ot::identifier::Generic>>;

    static constexpr auto chain_{ot::blockchain::Type::UnitTest};
    static constexpr auto pkt_words_{
        "forum school old approve bubble warfare robust figure pact glance "
        "farm leg taxi sing ankle"};
    static constexpr auto pkt_passphrase_{"Password123#"};

    static std::optional<User> alice_s_;
    static std::optional<User> bob_s_;
    static std::optional<User> chris_s_;
    static HDAccountMap hd_acct_;
    static PCAccountMap pc_acct_;

    const User& alice_;
    const User& bob_;
    const User& chris_;

    auto Account(const User& user, ot::blockchain::Type chain) const noexcept
        -> const ot::blockchain::crypto::Account&;
    auto make_hd_account(const User& user, const Protocol type) noexcept
        -> void;
    auto make_pc_account(const User& local, const User& remote) noexcept
        -> void;

    BlockchainAccountStatus();
};
}  // namespace ottest
