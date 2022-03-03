// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <QString>
#include <QValidator>
#include <future>
#include <tuple>
#include <type_traits>
#include <utility>

#include "common/Client.hpp"
#include "common/Notary.hpp"
#include "integration/Helpers.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/otx/client/Pair.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/interface/qt/AccountActivity.hpp"
#include "opentxs/interface/qt/AmountValidator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "paymentcode/VectorsV3.hpp"

namespace ot = opentxs;

namespace ottest
{
constexpr auto issuer_{
    "ot2xuVPJDdweZvKLQD42UMCzhCmT3okn3W1PktLgCbmQLRnaKy848sX"};

class Test_AmountValidator : public Notary_fixture, public Client_fixture
{
public:
    struct ChainAmounts {
        ot::UnitType unittype_;
        std::vector<std::pair<ot::UnallocatedCString, ot::UnallocatedCString>>
            amounts_;
        std::vector<std::tuple<
            ot::UnallocatedCString,
            ot::UnallocatedCString,
            ot::UnallocatedCString>>
            invalid_amounts_;
    };

    Test_AmountValidator() noexcept {}
};

TEST_F(Test_AmountValidator, preconditions)
{
    StartNotarySession(0);

    const auto& server = ot_.NotarySession(0);
    const auto& session = StartClient(0);
    session.OTX().DisableAutoaccept();
    session.InternalClient().Pair().Stop().get();
    const auto seed = ImportBip39(session, GetVectors3().alice_.words_);

    EXPECT_FALSE(seed.empty());
    EXPECT_TRUE(SetIntroductionServer(session, server));

    const auto& issuer = CreateNym(session, "issuer", seed, 0);

    EXPECT_EQ(issuer.nym_id_->str(), issuer_);
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

TEST_F(Test_AmountValidator, decimals)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()]
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

TEST_F(Test_AmountValidator, fixup)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()]
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

TEST_F(Test_AmountValidator, revise)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()]
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

TEST_F(Test_AmountValidator, scale)
{
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()]
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

TEST_F(Test_AmountValidator, validate)
{
    static const auto chains = std::vector<ChainAmounts>{
        {ot::UnitType::Btc,
         {{u8"0", u8"0 ₿"},
          {u8"1", u8"1 ₿"},
          {u8"10", u8"10 ₿"},
          {u8"25", u8"25 ₿"},
          {u8"300", u8"300 ₿"},
          {u8"4567", u8"4,567 ₿"},
          {u8"50000", u8"50,000 ₿"},
          {u8"678901", u8"678,901 ₿"},
          {u8"7000000", u8"7,000,000 ₿"},
          {u8"1000000000000000001", u8"1,000,000,000,000,000,001 ₿"},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"
           u8"269984665640564039457584007913129639935",
           u8"115,792,089,237,316,195,423,570,985,008,687,907,853,"
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935 ₿"},
          {u8".1", u8"0.1 ₿"},
          {u8".12345678", u8"0.123\u202F456\u202F78 ₿"},
          {u8"0.12345678", u8"0.123\u202F456\u202F78 ₿"},
          {u8"74.99999448", u8"74.999\u202F994\u202F48 ₿"},
          {u8"86.00002652", u8"86.000\u202F026\u202F52 ₿"},
          {u8"89.99999684", u8"89.999\u202F996\u202F84 ₿"},
          {u8"100.0000495", u8"100.000\u202F049\u202F5 ₿"}},
         {
             {u8"1.234567890",
              u8"1.234\u202F567\u202F890 ₿",
              u8"1.234\u202F567\u202F89 ₿"},
         }},
        {ot::UnitType::Ltc,
         {{u8"0", u8"0 Ł"},
          {u8"1", u8"1 Ł"},
          {u8"10", u8"10 Ł"},
          {u8"25", u8"25 Ł"},
          {u8"300", u8"300 Ł"},
          {u8"4567", u8"4,567 Ł"},
          {u8"50000", u8"50,000 Ł"},
          {u8"678901", u8"678,901 Ł"},
          {u8"7000000", u8"7,000,000 Ł"},
          {u8"1000000000000000001", u8"1,000,000,000,000,000,001 Ł"},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"
           u8"269984665640564039457584007913129639935",
           u8"115,792,089,237,316,195,423,570,985,008,687,907,853,"
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935 Ł"},
          {u8".1", u8"0.1 Ł"},
          {u8".123456", u8"0.123\u202F456 Ł"},
          {u8"0.123456", u8"0.123\u202F456 Ł"},
          {u8"74.999448", u8"74.999\u202F448 Ł"},
          {u8"86.002652", u8"86.002\u202F652 Ł"},
          {u8"89.999684", u8"89.999\u202F684 Ł"},
          {u8"100.00495", u8"100.004\u202F95 Ł"}},
         {
             {u8"1.234567890",
              u8"1.234\u202F567\u202F890 Ł",
              u8"1.234\u202F567 Ł"},
         }},
        {ot::UnitType::Pkt,
         {{u8"0", u8"0 PKT"},
          {u8"1", u8"1 PKT"},
          {u8"10", u8"10 PKT"},
          {u8"25", u8"25 PKT"},
          {u8"300", u8"300 PKT"},
          {u8"4567", u8"4,567 PKT"},
          {u8"50000", u8"50,000 PKT"},
          {u8"678901", u8"678,901 PKT"},
          {u8"7000000", u8"7,000,000 PKT"},
          {u8"1000000000000000001", u8"1,000,000,000,000,000,001 PKT"},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"
           u8"269984665640564039457584007913129639935",
           u8"115,792,089,237,316,195,423,570,985,008,687,907,853,"
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935 PKT"},
          {u8".1", u8"0.1 PKT"},
          {u8".12345678901", u8"0.123\u202F456\u202F789\u202F01 PKT"},
          {u8"0.12345678901", u8"0.123\u202F456\u202F789\u202F01 PKT"},
          {u8"74.99999448", u8"74.999\u202F994\u202F48 PKT"},
          {u8"86.00002652", u8"86.000\u202F026\u202F52 PKT"},
          {u8"89.99999684", u8"89.999\u202F996\u202F84 PKT"},
          {u8"100.0000495", u8"100.000\u202F049\u202F5 PKT"}},
         {
             {u8"1.234567890123",
              u8"1.234\u202F567\u202F890\u202F123 PKT",
              u8"1.234\u202F567\u202F890\u202F12 PKT"},
         }},
        {ot::UnitType::Usd,
         {{u8"0.00", u8"$0.00"},
          {u8"1.00", u8"$1.00"},
          {u8"10.00", u8"$10.00"},
          {u8"25.00", u8"$25.00"},
          {u8"300.00", u8"$300.00"},
          {u8"4567.00", u8"$4,567.00"},
          {u8"50000.00", u8"$50,000.00"},
          {u8"678901.00", u8"$678,901.00"},
          {u8"7000000.00", u8"$7,000,000.00"},
          {u8"1000000000000000001.00", u8"$1,000,000,000,000,000,001.00"},
          // Maximum value suported by Amount.
          {u8"115792089237316195423570985008687907853"
           u8"269984665640564039457584007913129639935.00",
           u8"$115,792,089,237,316,195,423,570,985,008,687,907,853,"
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935.00"},
          {u8".10", u8"$0.10"},
          {u8".123", u8"$0.123"},
          {u8"0.10", u8"$0.10"},
          {u8"0.123", u8"$0.123"},
          {u8"1.23", u8"$1.23"},
          {u8"74.999", u8"$74.999"},
          {u8"86.000", u8"$86.000"},
          {u8"89.999", u8"$89.999"},
          {u8"100.000", u8"$100.000"}},
         {
             {u8".1", u8"$0.10", u8"$0.1"},
             {u8"0.1", u8"$0.10", u8"$0.1"},
             {u8"1.2345", u8"$1.234\u202F5", u8"$1.234"},
         }},
    };

    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    for (const auto& chain : chains) {
        const auto* activity = session.UI().AccountActivityQt(
            issuer.nym_id_,
            opentxs::Identifier::Factory(
                registered_accounts_[issuer.nym_id_->str()]
                                    [account_index_[chain.unittype_]]));

        auto& amountValidator = *activity->getAmountValidator();
        amountValidator.setScale(0);

        auto pos = 0;
        for (const auto& [input, valid] : chain.amounts_) {
            auto inputString = QString{input.c_str()};
            auto validated = amountValidator.validate(inputString, pos);

            EXPECT_EQ(validated, QValidator::State::Acceptable);
            EXPECT_EQ(inputString.toStdString(), valid);
        }

        for (const auto& [input, expected, valid] : chain.invalid_amounts_) {
            auto inputString = QString{input.c_str()};
            auto validated = amountValidator.validate(inputString, pos);

            EXPECT_EQ(validated, QValidator::State::Invalid);
            EXPECT_NE(inputString.toStdString(), expected);
            EXPECT_EQ(inputString.toStdString(), valid);
        }
    }
}

TEST_F(Test_AmountValidator, cleanup)
{
    CleanupClient();
    CleanupNotary();
}
}  // namespace ottest
