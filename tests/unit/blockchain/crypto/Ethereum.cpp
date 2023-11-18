// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <string_view>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/common/OneClientSession.hpp"
#include "ottest/fixtures/common/User.hpp"

namespace ottest
{
using namespace std::literals;

User user_{GetPaymentCodeVector3().alice_.words_, "User"};

TEST_F(OneClientSession, create_account)
{
    user_.init(client_1_);

    EXPECT_EQ(
        user_.payment_code_, GetPaymentCodeVector3().alice_.payment_code_);

    const auto& api = *user_.api_;
    const auto& nymID = user_.nym_id_;
    const auto reason = api.Factory().PasswordPrompt(__func__);
    constexpr auto chain = opentxs::blockchain::Type::Ethereum_sepolia;
    constexpr auto subchain = opentxs::blockchain::crypto::Subchain::None;
    constexpr auto subaccountType =
        opentxs::blockchain::crypto::SubaccountType::Imported;
    constexpr auto addressStyle =
        opentxs::blockchain::crypto::AddressStyle::ethereum_account;
    constexpr auto expectedAccount =
        "0x4940214377E9D02c6bC04c11d7659D7645536A53"sv;
    const auto subaccountID = api.Crypto().Blockchain().NewEthereumSubaccount(
        nymID, ot::blockchain::crypto::HDProtocol::BIP_44, chain, reason);

    EXPECT_FALSE(subaccountID.empty());

    auto& subaccount = api.Crypto()
                           .Blockchain()
                           .Account(nymID, chain)
                           .Subaccount(subaccountID)
                           .asImported()
                           .asEthereum();

    EXPECT_TRUE(subaccount.IsValid());

    const auto subchains = subaccount.AllowedSubchains();

    ASSERT_EQ(subchains.size(), 1);
    EXPECT_EQ(*subchains.cbegin(), subchain);

    const auto& element = subaccount.BalanceElement(subchain, 0);
    const auto address = api.Crypto().Blockchain().EncodeAddress(
        addressStyle, chain, element.Key());

    EXPECT_EQ(address, expectedAccount);
    EXPECT_EQ(element.Index(), 0);
    EXPECT_EQ(element.Address(addressStyle), expectedAccount);
    EXPECT_EQ(subaccount.ID(), subaccountID);
    EXPECT_EQ(subaccount.Type(), subaccountType);
    EXPECT_EQ(subaccount.Balance(), 0);
    EXPECT_TRUE(subaccount.KnownIncoming({}).empty());
    EXPECT_TRUE(subaccount.KnownOutgoing({}).empty());
    EXPECT_TRUE(subaccount.MissingOutgoing({}).empty());
    EXPECT_EQ(subaccount.NextOutgoing(), 0);

    EXPECT_TRUE(subaccount.UpdateBalance(2));
    EXPECT_EQ(subaccount.Balance(), 2);
}
}  // namespace ottest
