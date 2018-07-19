/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/opentxs.hpp"
#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

class Test_Bip47 : public ::testing::Test
{
public:
  std::string A_Fingerprint, A_Mnemonic, A_NotificationAddress, A_NymID, SharedSecret_0;
  std::string B_Paycode = "PM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjBxteQ5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97";

  std::string B_Fingerprint, B_Mnemonic, B_NotificationAddress, B_NymID;
  std::string A_Paycode = "PM8TJTLJbPRGxSbc8EJi42Wrr6QbNSaSSVJ5Y3E4pbCYiTHUskHg13935Ubb7q8tx9GVbh2UuRnBc3WSyJHhUrw8KhprKnn9eDznYGieTzFcwQRya4GA";

  Test_Bip47() {
      A_Mnemonic = "response seminar brave tip suit recall often sound stick owner lottery motion";
      A_Fingerprint = opentxs::OT::App().API().Exec().Wallet_ImportSeed(A_Mnemonic, "");
      A_NotificationAddress = "1JDdmqFLhpzcUwPeinhJbUPw4Co3aWLyzW";
      A_NymID = opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "A", A_Fingerprint, 0);

      B_Mnemonic = "reward upper indicate eight swift arch injury crystal super wrestle already dentist";
      B_Fingerprint = opentxs::OT::App().API().Exec().Wallet_ImportSeed(B_Mnemonic, "");
      B_NotificationAddress = "1ChvUUvht2hUQufHBXF8NgLhW8SwE2ecGV";
      B_NymID = opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "B", B_Fingerprint, 0);
  }
      
};

TEST_F(Test_Bip47, test_NotificationAddress)
{
    ASSERT_TRUE(true);
    const ConstNym A_Nym = opentxs::OT::App().Wallet().Nym(Identifier(A_NymID));
    ASSERT_STREQ(A_NotificationAddress.c_str(), OT::App().Crypto().BIP47().NotificationAddress(*A_Nym, proto::CITEMTYPE_BTC).c_str());

    ASSERT_TRUE(true);
    const ConstNym B_Nym = opentxs::OT::App().Wallet().Nym(Identifier(B_NymID));
    ASSERT_STREQ(B_NotificationAddress.c_str(), OT::App().Crypto().BIP47().NotificationAddress(*B_Nym, proto::CITEMTYPE_BTC).c_str());
}

TEST_F(Test_Bip47, test_ecdh_vectors)
{
    const ConstNym A_Nym = opentxs::OT::App().Wallet().Nym(Identifier(A_NymID));
    const ConstNym B_Nym = opentxs::OT::App().Wallet().Nym(Identifier(B_NymID));

    std::cout << "Constructing A payment code?";
    OTPaymentCode A_PaymentCode = PaymentCode::Factory(A_Fingerprint, 0, 1);
    OTPaymentCode B_PaymentCode = PaymentCode::Factory(B_Fingerprint, 0, 1);    

    auto acc_a = OT::App().Crypto().BIP47().AccountSource(*A_Nym, proto::CITEMTYPE_BTC);
    auto acc_b = OT::App().Crypto().BIP47().AccountSource(*B_Nym, proto::CITEMTYPE_BTC);

    auto [a0_, a0] = OT::App().Crypto().BIP47().LocalPaymentCode(acc_a, 0);
    auto [a1_, a1] = OT::App().Crypto().BIP47().LocalPaymentCode(acc_a, 1);
    auto [a2_, a2] = OT::App().Crypto().BIP47().LocalPaymentCode(acc_a, 2);
    auto [b0_, b0] = OT::App().Crypto().BIP47().LocalPaymentCode(acc_b, 0);
    auto [b1_, b1] = OT::App().Crypto().BIP47().LocalPaymentCode(acc_b, 1);
    auto [b2_, b2] = OT::App().Crypto().BIP47().LocalPaymentCode(acc_b, 2);
    ASSERT_TRUE(a0_); ASSERT_TRUE(a1_); ASSERT_TRUE(a2_);
    ASSERT_TRUE(b0_); ASSERT_TRUE(b1_); ASSERT_TRUE(b2_);

    EXPECT_STREQ("8D6A8ECD8EE5E0042AD0CB56E3A971C760B5145C3917A8E7BEAF0ED92D7A520C", a0.asHex().c_str());
    EXPECT_STREQ("60E44A7849045946C177FA76A7039F3600F35691D4E8D9CA0F56C2AEE503A751", a1.asHex().c_str());
    EXPECT_STREQ("20E8A1695BD9E048AA436CFB6446D8FABAAB8B53C4D9FE633E24A8AEFD801B64", a2.asHex().c_str());
    EXPECT_STREQ("04448FD1BE0C9C13A5CA0B530E464B619DC091B299B98C5CAB9978B32B4A1B8B", b0.asHex().c_str());
    EXPECT_STREQ("6BFA917E4C44349BFDF46346D389BF73A18CEC6BC544CE9F337E14721F06107B", b1.asHex().c_str());
    EXPECT_STREQ("46D32FBEE043D8EE176FE85A18DA92557EE00B189B533FCE2340E4745C4B7B8C", b2.asHex().c_str());
    
    auto [A0_, A0] = OT::App().Crypto().BIP47().RemotePaymentCode(A_PaymentCode, 0);
    auto [A1_, A1] = OT::App().Crypto().BIP47().RemotePaymentCode(A_PaymentCode, 1);
    auto [A2_, A2] = OT::App().Crypto().BIP47().RemotePaymentCode(A_PaymentCode, 2);
    auto [B0_, B0] = OT::App().Crypto().BIP47().RemotePaymentCode(B_PaymentCode, 0);
    auto [B1_, B1] = OT::App().Crypto().BIP47().RemotePaymentCode(B_PaymentCode, 1);
    auto [B2_, B2] = OT::App().Crypto().BIP47().RemotePaymentCode(B_PaymentCode, 2);
    ASSERT_TRUE(B0_); ASSERT_TRUE(B1_);
    
    EXPECT_STREQ("0353883A146A23F988E0F381A9507CBDB3E3130CD81B3CE26DAF2AF088724CE683", A0->asHex().c_str());
    EXPECT_STREQ("036C6CC9446891A80184188533FE81D56110432A160E74B94D5E2294B9F13DAAE6", A1->asHex().c_str());
    
    EXPECT_STREQ("024CE8E3B04EA205FF49F529950616C3DB615B1E37753858CC60C1CE64D17E2AD8", B0->asHex().c_str());
    EXPECT_STREQ("03E092E58581CF950FF9C8FC64395471733E13F97DEDAC0044EBD7D60CCC1EEA4D", B1->asHex().c_str());
    EXPECT_STREQ("029B5F290EF2F98A0462EC691F5CC3AE939325F7577FCAF06CFC3B8FC249402156", B2->asHex().c_str());

    auto [S0_, S0] = OT::App().Crypto().BIP47().SecretPoint(a0, B0);
    auto [S0_prime_, S0_prime] = OT::App().Crypto().BIP47().SecretPoint(b0, A0);

    ASSERT_TRUE(S0_); ASSERT_TRUE(S0_prime_);
    EXPECT_STREQ(S0.asHex().c_str(), S0_prime.asHex().c_str());
    EXPECT_STREQ("F5BB84706EE366052471E6139E6A9A969D586E5FE6471A9B96C3D8CAEFE86FEF", S0.asHex().c_str());
    EXPECT_STREQ("F5BB84706EE366052471E6139E6A9A969D586E5FE6471A9B96C3D8CAEFE86FEF", S0_prime.asHex().c_str());
    
    auto [S1_, S1] = OT::App().Crypto().BIP47().SecretPoint(a0, B1);
    auto [S1_prime_, S1_prime] = OT::App().Crypto().BIP47().SecretPoint(b1, A0);

    ASSERT_TRUE(S1_); ASSERT_TRUE(S1_prime_);
    EXPECT_STREQ(S1.asHex().c_str(), S1_prime.asHex().c_str());
    EXPECT_STREQ("ADFB9B18EE1C4460852806A8780802096D67A8C1766222598DC801076BEB0B4D", S1.asHex().c_str());
    EXPECT_STREQ("ADFB9B18EE1C4460852806A8780802096D67A8C1766222598DC801076BEB0B4D", S1_prime.asHex().c_str());

    auto [S2_, S2] = OT::App().Crypto().BIP47().SecretPoint(a0, B2);
    auto [S2_prime_, S2_prime] = OT::App().Crypto().BIP47().SecretPoint(b2, A0);

    ASSERT_TRUE(S2_); ASSERT_TRUE(S2_prime_);
    EXPECT_STREQ(S2.asHex().c_str(), S2_prime.asHex().c_str());
    EXPECT_STREQ("79E860C3EB885723BB5A1D54E5CECB7DF5DC33B1D56802906762622FA3C18EE5", S2.asHex().c_str());
    EXPECT_STREQ("79E860C3EB885723BB5A1D54E5CECB7DF5DC33B1D56802906762622FA3C18EE5", S2_prime.asHex().c_str());
}

TEST_F(Test_Bip47, test_shared_addresses)
{
    const ConstNym A_Nym = opentxs::OT::App().Wallet().Nym(Identifier(A_NymID));
    const ConstNym B_Nym = opentxs::OT::App().Wallet().Nym(Identifier(B_NymID));

    OTPaymentCode A_PaymentCode = PaymentCode::Factory(A_Fingerprint, 0, 1);
    OTPaymentCode B_PaymentCode = PaymentCode::Factory(B_Fingerprint, 0, 1);

    auto A_Send_0 = OT::App().Crypto().BIP47().OutgoingPubkey(*A_Nym, B_PaymentCode, proto::CITEMTYPE_BTC, 0);
    auto B_Receive_0 = OT::App().Crypto().BIP47().IncomingPubkey(*B_Nym, A_PaymentCode, proto::CITEMTYPE_BTC, 0);

    auto A_Send_1 = OT::App().Crypto().BIP47().OutgoingPubkey(*A_Nym, B_PaymentCode, proto::CITEMTYPE_BTC, 1);
    auto B_Receive_1 = OT::App().Crypto().BIP47().IncomingPubkey(*B_Nym, A_PaymentCode, proto::CITEMTYPE_BTC, 1);
    
    auto B_Send_0 = OT::App().Crypto().BIP47().OutgoingPubkey(*B_Nym, A_PaymentCode, proto::CITEMTYPE_BTC, 0);
    auto A_Receive_0 = OT::App().Crypto().BIP47().IncomingPubkey(*A_Nym, B_PaymentCode, proto::CITEMTYPE_BTC, 0);

    auto B_Send_1 = OT::App().Crypto().BIP47().OutgoingPubkey(*B_Nym, A_PaymentCode, proto::CITEMTYPE_BTC, 1);
    auto A_Receive_1 = OT::App().Crypto().BIP47().IncomingPubkey(*A_Nym, B_PaymentCode, proto::CITEMTYPE_BTC, 1);

    EXPECT_STREQ("141fi7TY3h936vRUKh1qfUZr8rSBuYbVBK", OT::App().Blockchain().CalculateAddress(A_Send_0, proto::CITEMTYPE_BTC).c_str());
    EXPECT_STREQ("141fi7TY3h936vRUKh1qfUZr8rSBuYbVBK", OT::App().Blockchain().CalculateAddress(B_Receive_0, proto::CITEMTYPE_BTC).c_str());

    EXPECT_STREQ("12u3Uued2fuko2nY4SoSFGCoGLCBUGPkk6", OT::App().Blockchain().CalculateAddress(A_Send_1, proto::CITEMTYPE_BTC).c_str());
    EXPECT_STREQ("12u3Uued2fuko2nY4SoSFGCoGLCBUGPkk6", OT::App().Blockchain().CalculateAddress(B_Receive_1, proto::CITEMTYPE_BTC).c_str());
    
    EXPECT_STREQ("17SSoP6pwU1yq6fTATEQ7gLMDWiycm68VT", OT::App().Blockchain().CalculateAddress(B_Send_0, proto::CITEMTYPE_BTC).c_str());
    EXPECT_STREQ("17SSoP6pwU1yq6fTATEQ7gLMDWiycm68VT", OT::App().Blockchain().CalculateAddress(A_Receive_0, proto::CITEMTYPE_BTC).c_str());

    EXPECT_STREQ("1KNFAqYPoiy29rTQF44YT3v9tvRJYi15Xf", OT::App().Blockchain().CalculateAddress(B_Send_1, proto::CITEMTYPE_BTC).c_str());
    EXPECT_STREQ("1KNFAqYPoiy29rTQF44YT3v9tvRJYi15Xf", OT::App().Blockchain().CalculateAddress(A_Receive_1, proto::CITEMTYPE_BTC).c_str());
}

} // namespace
