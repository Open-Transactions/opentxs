// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/Qt.hpp>
#include <opentxs/opentxs.hpp>
#include <QString>
#include <QValidator>
#include <string_view>
#include <tuple>
#include <utility>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/common/Notary.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/AmountValidator.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

constexpr auto issuer_{
    "ot2xuVPJDdweZvKLQD42UMCzhCmT3okn3W1PktLgCbmQLRnaKy848sX"};

TEST_F(AmountValidator, preconditions)
{
    StartNotarySession(0);

    const auto& server = ot_.NotarySession(0);
    const auto& session = StartClient(0);
    session.OTX().DisableAutoaccept();
    StopPair(session);
    const auto seed =
        ImportBip39(session, GetPaymentCodeVector3().alice_.words_);

    EXPECT_FALSE(seed.empty());
    EXPECT_TRUE(SetIntroductionServer(session, server));

    const auto& issuer = CreateNym(session, "issuer", seed, 0);

    EXPECT_EQ(issuer.nym_id_.asBase58(ot_.Crypto()), issuer_);
    EXPECT_TRUE(RegisterNym(server, issuer));

    {
        const auto& def = ot::display::GetDefinition(ot::UnitType::Regtest);
        const auto unit = IssueUnit(
            server,
            issuer,
            "decimals-" + ot::UnallocatedCString{def.ShortName()},
            "Test",
            ot::UnitType::Regtest,
            def);

        EXPECT_FALSE(unit.empty());
    }

    {
        const auto& def = ot::display::GetDefinition(ot::UnitType::Btc);
        const auto unit = IssueUnit(
            server,
            issuer,
            "revise-" + ot::UnallocatedCString{def.ShortName()},
            "Test",
            ot::UnitType::Btc,
            def);

        EXPECT_FALSE(unit.empty());
    }

    {
        const auto& def = ot::display::GetDefinition(ot::UnitType::Bch);
        const auto unit = IssueUnit(
            server,
            issuer,
            "revise-" + ot::UnallocatedCString{def.ShortName()},
            "Test",
            ot::UnitType::Bch,
            def);

        EXPECT_FALSE(unit.empty());
    }

    {
        const auto& def = ot::display::GetDefinition(ot::UnitType::Ltc);
        const auto unit = IssueUnit(
            server,
            issuer,
            "revise-" + ot::UnallocatedCString{def.ShortName()},
            "Test",
            ot::UnitType::Ltc,
            def);

        EXPECT_FALSE(unit.empty());
    }

    {
        const auto& def = ot::display::GetDefinition(ot::UnitType::Pkt);
        const auto unit = IssueUnit(
            server,
            issuer,
            "revise-" + ot::UnallocatedCString{def.ShortName()},
            "Test",
            ot::UnitType::Pkt,
            def);

        EXPECT_FALSE(unit.empty());
    }

    {
        const auto& def = ot::display::GetDefinition(ot::UnitType::Usd);
        const auto unit = IssueUnit(
            server,
            issuer,
            "revise-" + ot::UnallocatedCString{def.ShortName()},
            "Test",
            ot::UnitType::Usd,
            def);

        EXPECT_FALSE(unit.empty());
    }
}

TEST_F(AmountValidator, decimals)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        session.Factory().AccountIDFromBase58(
            registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())]
                                [account_index_[ot::UnitType::Regtest]]));

    auto& amountValidator = *activity->getAmountValidator();
    amountValidator.setScale(0);

    // Default max decimals is 8.
    auto inputString = QString{"1.234567890"};
    auto pos = 0;
    auto validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Invalid);

    inputString = QString{"1.23456789"};
    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 units");

    inputString = QString{"1.234567890"};
    amountValidator.setMaxDecimals(4);

    EXPECT_EQ(amountValidator.getMaxDecimals(), 4);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Invalid);

    inputString = QString{"1.2345"};
    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F5 units");

    inputString = QString{"1.2"};
    amountValidator.setMinDecimals(3);

    EXPECT_EQ(amountValidator.getMinDecimals(), 3);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Invalid);

    inputString = QString{"1.234"};
    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.234 units");
}

TEST_F(AmountValidator, fixup)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        session.Factory().AccountIDFromBase58(
            registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())]
                                [account_index_[ot::UnitType::Regtest]]));

    auto& amountValidator = *activity->getAmountValidator();
    // Reset from previous test.
    amountValidator.setMinDecimals(0);
    amountValidator.setMaxDecimals(8);

    auto inputString = QString{"1.234567890"};
    amountValidator.fixup(inputString);

    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 units");

    amountValidator.setMinDecimals(4);
    inputString = QString{"1"};
    amountValidator.fixup(inputString);

    EXPECT_EQ(inputString.toStdString(), "1.000\u202F0 units");
}

TEST_F(AmountValidator, revise)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        session.Factory().AccountIDFromBase58(
            registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())]
                                [account_index_[ot::UnitType::Bch]]));

    auto& amountValidator = *activity->getAmountValidator();
    amountValidator.setScale(0);

    auto inputString = QString{"1.23456789"};
    auto pos = 0;
    auto validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 BCH");

    amountValidator.setScale(1);
    auto revised = amountValidator.revise(inputString, 0);
    EXPECT_EQ(revised.toStdString(), "1,234.567\u202F89 mBCH");

    amountValidator.setScale(0);
    revised = amountValidator.revise(revised, 1);
    EXPECT_EQ(revised.toStdString(), "1.234\u202F567\u202F89 BCH");

    amountValidator.setScale(2);
    revised = amountValidator.revise(revised, 0);
    EXPECT_EQ(revised.toStdString(), "1,234,567.89 bits");

    amountValidator.setScale(3);
    revised = amountValidator.revise(revised, 2);
    EXPECT_EQ(revised.toStdString(), "1,234,567.89 μBCH");

    amountValidator.setScale(4);
    revised = amountValidator.revise(revised, 3);
    EXPECT_EQ(revised.toStdString(), "123,456,789 satoshis");
}

TEST_F(AmountValidator, scale)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        session.Factory().AccountIDFromBase58(
            registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())]
                                [account_index_[ot::UnitType::Bch]]));

    auto& amountValidator = *activity->getAmountValidator();
    amountValidator.setScale(0);

    auto inputString = QString{"1.23456789"};
    auto pos = 0;
    auto validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 BCH");

    inputString = QString{"1.23456"};
    amountValidator.setScale(1);

    EXPECT_EQ(amountValidator.getScale(), 1);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F56 mBCH");

    inputString = QString{"1.23"};
    amountValidator.setScale(2);

    EXPECT_EQ(amountValidator.getScale(), 2);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.23 bits");

    inputString = QString{"1.23"};
    amountValidator.setScale(3);

    EXPECT_EQ(amountValidator.getScale(), 3);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.23 μBCH");

    inputString = QString{"1.23456789"};
    amountValidator.setScale(4);

    EXPECT_EQ(amountValidator.getScale(), 4);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 satoshis");
}

TEST_F(AmountValidator, validate)
{
    static const auto chains = std::vector<ChainAmounts>{
        {ot::UnitType::Btc,
         {{u8"0"sv, u8"0 ₿"sv},
          {u8"1"sv, u8"1 ₿"sv},
          {u8"10"sv, u8"10 ₿"sv},
          {u8"25"sv, u8"25 ₿"sv},
          {u8"300"sv, u8"300 ₿"sv},
          {u8"4567"sv, u8"4,567 ₿"sv},
          {u8"50000"sv, u8"50,000 ₿"sv},
          {u8"678901"sv, u8"678,901 ₿"sv},
          {u8"7000000"sv, u8"7,000,000 ₿"sv},
          {u8"1000000000000000001"sv, u8"1,000,000,000,000,000,001 ₿"sv},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"sv
           u8"269984665640564039457584007913129639935"sv,
           u8"115,792,089,237,316,195,423,570,985,008,687,907,853,"sv
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935 ₿"sv},
          {u8".1"sv, u8"0.1 ₿"sv},
          {u8".12345678"sv, u8"0.123\u202F456\u202F78 ₿"sv},
          {u8"0.12345678"sv, u8"0.123\u202F456\u202F78 ₿"sv},
          {u8"74.99999448"sv, u8"74.999\u202F994\u202F48 ₿"sv},
          {u8"86.00002652"sv, u8"86.000\u202F026\u202F52 ₿"sv},
          {u8"89.99999684"sv, u8"89.999\u202F996\u202F84 ₿"sv},
          {u8"100.0000495"sv, u8"100.000\u202F049\u202F5 ₿"sv}},
         {
             {u8"1.234567890"sv,
              u8"1.234\u202F567\u202F890 ₿"sv,
              u8"1.234\u202F567\u202F89 ₿"sv},
         }},
        {ot::UnitType::Ltc,
         {{u8"0"sv, u8"0 Ł"sv},
          {u8"1"sv, u8"1 Ł"sv},
          {u8"10"sv, u8"10 Ł"sv},
          {u8"25"sv, u8"25 Ł"sv},
          {u8"300"sv, u8"300 Ł"sv},
          {u8"4567"sv, u8"4,567 Ł"sv},
          {u8"50000"sv, u8"50,000 Ł"sv},
          {u8"678901"sv, u8"678,901 Ł"sv},
          {u8"7000000"sv, u8"7,000,000 Ł"sv},
          {u8"1000000000000000001"sv, u8"1,000,000,000,000,000,001 Ł"sv},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"sv
           u8"269984665640564039457584007913129639935"sv,
           u8"115,792,089,237,316,195,423,570,985,008,687,907,853,"sv
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935 Ł"sv},
          {u8".1"sv, u8"0.1 Ł"sv},
          {u8".123456"sv, u8"0.123\u202F456 Ł"sv},
          {u8"0.123456"sv, u8"0.123\u202F456 Ł"sv},
          {u8"74.999448"sv, u8"74.999\u202F448 Ł"sv},
          {u8"86.002652"sv, u8"86.002\u202F652 Ł"sv},
          {u8"89.999684"sv, u8"89.999\u202F684 Ł"sv},
          {u8"100.00495"sv, u8"100.004\u202F95 Ł"sv}},
         {
             {u8"1.234567890"sv,
              u8"1.234\u202F567\u202F890 Ł"sv,
              u8"1.234\u202F567 Ł"sv},
         }},
        {ot::UnitType::Pkt,
         {{u8"0"sv, u8"0 PKT"sv},
          {u8"1"sv, u8"1 PKT"sv},
          {u8"10"sv, u8"10 PKT"sv},
          {u8"25"sv, u8"25 PKT"sv},
          {u8"300"sv, u8"300 PKT"sv},
          {u8"4567"sv, u8"4,567 PKT"sv},
          {u8"50000"sv, u8"50,000 PKT"sv},
          {u8"678901"sv, u8"678,901 PKT"sv},
          {u8"7000000"sv, u8"7,000,000 PKT"sv},
          {u8"1000000000000000001"sv, u8"1,000,000,000,000,000,001 PKT"sv},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"sv
           u8"269984665640564039457584007913129639935"sv,
           u8"115,792,089,237,316,195,423,570,985,008,687,907,853,"sv
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935 PKT"sv},
          {u8".1"sv, u8"0.1 PKT"sv},
          {u8".12345678901"sv, u8"0.123\u202F456\u202F789\u202F01 PKT"sv},
          {u8"0.12345678901"sv, u8"0.123\u202F456\u202F789\u202F01 PKT"sv},
          {u8"74.99999448"sv, u8"74.999\u202F994\u202F48 PKT"sv},
          {u8"86.00002652"sv, u8"86.000\u202F026\u202F52 PKT"sv},
          {u8"89.99999684"sv, u8"89.999\u202F996\u202F84 PKT"sv},
          {u8"100.0000495"sv, u8"100.000\u202F049\u202F5 PKT"sv}},
         {
             {u8"1.234567890123"sv,
              u8"1.234\u202F567\u202F890\u202F123 PKT"sv,
              u8"1.234\u202F567\u202F890\u202F12 PKT"sv},
         }},
        {ot::UnitType::Usd,
         {{u8"0.00"sv, u8"$0.00"sv},
          {u8"1.00"sv, u8"$1.00"sv},
          {u8"10.00"sv, u8"$10.00"sv},
          {u8"25.00"sv, u8"$25.00"sv},
          {u8"300.00"sv, u8"$300.00"sv},
          {u8"4567.00"sv, u8"$4,567.00"sv},
          {u8"50000.00"sv, u8"$50,000.00"sv},
          {u8"678901.00"sv, u8"$678,901.00"sv},
          {u8"7000000.00"sv, u8"$7,000,000.00"sv},
          {u8"1000000000000000001.00"sv, u8"$1,000,000,000,000,000,001.00"sv},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"sv

           u8"269984665640564039457584007913129639935.00"sv,

           u8"$115,792,089,237,316,195,423,570,985,008,687,907,853,"sv

           u8"269,984,665,640,564,039,457,584,007,913,129,639,935.00"sv},
          {u8".10"sv, u8"$0.10"sv},
          {u8".123"sv, u8"$0.123"sv},
          {u8"0.10"sv, u8"$0.10"sv},
          {u8"0.123"sv, u8"$0.123"sv},
          {u8"1.23"sv, u8"$1.23"sv},
          {u8"74.999"sv, u8"$74.999"sv},
          {u8"86.000"sv, u8"$86.000"sv},
          {u8"89.999"sv, u8"$89.999"sv},
          {u8"100.000"sv, u8"$100.000"sv}},
         {
             {u8".1"sv, u8"$0.10"sv, u8"$0.1"sv},
             {u8"0.1"sv, u8"$0.10"sv, u8"$0.1"sv},
             {u8"1.2345"sv, u8"$1.234\u202F5"sv, u8"$1.234"sv},
         }},
    };

    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    for (const auto& chain : chains) {
        const auto* activity = session.UI().AccountActivityQt(
            issuer.nym_id_,
            session.Factory().AccountIDFromBase58(
                registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())]
                                    [account_index_[chain.unittype_]]));

        auto& amountValidator = *activity->getAmountValidator();
        amountValidator.setScale(0);

        auto pos = 0;
        for (const auto& [input, valid] : chain.amounts_) {
            auto inputString =
                QString::fromUtf8(input.data(), static_cast<int>(input.size()));
            auto validated = amountValidator.validate(inputString, pos);

            EXPECT_EQ(validated, QValidator::State::Acceptable);
            EXPECT_EQ(inputString.toStdString(), valid);
        }

        for (const auto& [input, expected, valid] : chain.invalid_amounts_) {
            auto inputString =
                QString::fromUtf8(input.data(), static_cast<int>(input.size()));
            auto validated = amountValidator.validate(inputString, pos);

            EXPECT_EQ(validated, QValidator::State::Invalid);
            EXPECT_NE(inputString.toStdString(), expected);
            EXPECT_EQ(inputString.toStdString(), valid);
        }
    }
}

TEST_F(AmountValidator, cleanup)
{
    CleanupClient();
    CleanupNotary();
}
}  // namespace ottest
