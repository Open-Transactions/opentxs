// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "internal/otx/common/Cheque.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstring>

#include "internal/core/Factory.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
Cheque::Cheque(const api::Session& api)
    : ot_super(api)
    , m_lAmount(0)
    , m_strMemo(String::Factory())
    , m_RECIPIENT_NYM_ID(api_.Factory().NymID())
    , m_bHasRecipient(false)
    , m_REMITTER_NYM_ID(api_.Factory().NymID())
    , m_REMITTER_ACCT_ID(api_.Factory().Identifier())
    , m_bHasRemitter(false)
{
    InitCheque();
}

Cheque::Cheque(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    : ot_super(api, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lAmount(0)
    , m_strMemo(String::Factory())
    , m_RECIPIENT_NYM_ID(api_.Factory().NymID())
    , m_bHasRecipient(false)
    , m_REMITTER_NYM_ID(api_.Factory().NymID())
    , m_REMITTER_ACCT_ID(api_.Factory().Identifier())
    , m_bHasRemitter(false)
{
    InitCheque();
}

void Cheque::UpdateContents([[maybe_unused]] const PasswordPrompt& reason)
{
    auto INSTRUMENT_DEFINITION_ID =
             String::Factory(GetInstrumentDefinitionID()),
         NOTARY_ID = String::Factory(GetNotaryID()),
         SENDER_ACCT_ID = String::Factory(GetSenderAcctID()),
         SENDER_NYM_ID = String::Factory(GetSenderNymID()),
         RECIPIENT_NYM_ID = String::Factory(GetRecipientNymID()),
         REMITTER_NYM_ID = String::Factory(GetRemitterNymID()),
         REMITTER_ACCT_ID = String::Factory(GetRemitterAcctID());

    UnallocatedCString from = formatTimestamp(GetValidFrom());
    UnallocatedCString to = formatTimestamp(GetValidTo());

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("cheque");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("amount", [&] {
        auto buf = UnallocatedCString{};
        m_lAmount.Serialize(writer(buf));
        return buf;
    }());
    tag.add_attribute(
        "instrumentDefinitionID", INSTRUMENT_DEFINITION_ID->Get());
    tag.add_attribute("transactionNum", std::to_string(GetTransactionNum()));
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute("senderAcctID", SENDER_ACCT_ID->Get());
    tag.add_attribute("senderNymID", SENDER_NYM_ID->Get());
    tag.add_attribute("hasRecipient", formatBool(m_bHasRecipient));
    tag.add_attribute(
        "recipientNymID", m_bHasRecipient ? RECIPIENT_NYM_ID->Get() : "");
    tag.add_attribute("hasRemitter", formatBool(m_bHasRemitter));
    tag.add_attribute(
        "remitterNymID", m_bHasRemitter ? REMITTER_NYM_ID->Get() : "");
    tag.add_attribute(
        "remitterAcctID", m_bHasRemitter ? REMITTER_ACCT_ID->Get() : "");

    tag.add_attribute("validFrom", from);
    tag.add_attribute("validTo", to);

    if (m_strMemo->Exists() && m_strMemo->GetLength() > 2) {
        auto ascMemo = Armored::Factory(m_strMemo);
        tag.add_tag("memo", ascMemo->Get());
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate(String::Factory(str_result));
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto Cheque::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = Contract::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (!strcmp("cheque", xml->getNodeName())) {
        auto strHasRecipient =
            String::Factory(xml->getAttributeValue("hasRecipient"));
        m_bHasRecipient = strHasRecipient->Compare("true");

        auto strHasRemitter =
            String::Factory(xml->getAttributeValue("hasRemitter"));
        m_bHasRemitter = strHasRemitter->Compare("true");

        m_strVersion = String::Factory(xml->getAttributeValue("version"));
        m_lAmount = factory::Amount(xml->getAttributeValue("amount"));

        SetTransactionNum(
            String::StringToLong(xml->getAttributeValue("transactionNum")));

        const UnallocatedCString str_valid_from =
            xml->getAttributeValue("validFrom");
        const UnallocatedCString str_valid_to =
            xml->getAttributeValue("validTo");

        SetValidFrom(parseTimestamp(str_valid_from));
        SetValidTo(parseTimestamp(str_valid_to));

        auto strInstrumentDefinitionID = String::Factory(
                 xml->getAttributeValue("instrumentDefinitionID")),
             strNotaryID = String::Factory(xml->getAttributeValue("notaryID")),
             strSenderAcctID =
                 String::Factory(xml->getAttributeValue("senderAcctID")),
             strSenderNymID =
                 String::Factory(xml->getAttributeValue("senderNymID")),
             strRecipientNymID =
                 String::Factory(xml->getAttributeValue("recipientNymID")),
             strRemitterNymID =
                 String::Factory(xml->getAttributeValue("remitterNymID")),
             strRemitterAcctID =
                 String::Factory(xml->getAttributeValue("remitterAcctID"));

        const auto INSTRUMENT_DEFINITION_ID =
            api_.Factory().UnitID(strInstrumentDefinitionID);
        const auto NOTARY_ID = api_.Factory().ServerID(strNotaryID);
        const auto SENDER_ACCT_ID = api_.Factory().Identifier(strSenderAcctID);
        const auto SENDER_NYM_ID = api_.Factory().NymID(strSenderNymID);

        SetInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
        SetNotaryID(NOTARY_ID);
        SetSenderAcctID(SENDER_ACCT_ID);
        SetSenderNymID(SENDER_NYM_ID);

        // Recipient ID
        if (m_bHasRecipient)
            m_RECIPIENT_NYM_ID->SetString(strRecipientNymID);
        else
            m_RECIPIENT_NYM_ID->Release();

        // Remitter ID (for vouchers)
        if (m_bHasRemitter) {
            m_REMITTER_NYM_ID->SetString(strRemitterNymID);
            m_REMITTER_ACCT_ID->SetString(strRemitterAcctID);
        } else {
            m_REMITTER_NYM_ID->Release();
            m_REMITTER_ACCT_ID->Release();
        }
        {
            const auto unittype = api_.Wallet().CurrencyTypeBasedOnUnitType(
                INSTRUMENT_DEFINITION_ID);
            LogVerbose()(OT_PRETTY_CLASS())("Cheque Amount: ")(
                m_lAmount, unittype)(". Transaction Number: ")(
                m_lTransactionNum)(" Valid From: ")(
                str_valid_from)(" Valid To: ")(
                str_valid_to)(" InstrumentDefinitionID: ")(
                strInstrumentDefinitionID)(" NotaryID: ")(
                strNotaryID)(" senderAcctID: ")(
                strSenderAcctID)(" senderNymID: ")(
                strSenderNymID)(" Has Recipient? ")(
                m_bHasRecipient ? "Yes"
                                : "No")(". If yes, NymID of Recipient: ")(
                strRecipientNymID)(" Has Remitter? ")(
                m_bHasRemitter ? "Yes"
                               : "No")(". If yes, NymID/Acct of Remitter: ")(
                strRemitterNymID)(" / ")(strRemitterAcctID)
                .Flush();
        }
        nReturnVal = 1;
    } else if (!strcmp("memo", xml->getNodeName())) {
        if (!LoadEncodedTextField(xml, m_strMemo)) {
            LogError()(OT_PRETTY_CLASS())("Error: Memo field without value.")
                .Flush();
            return (-1);  // error condition
        }

        return 1;
    }

    return nReturnVal;
}

// You still need to re-sign the cheque after doing this.
void Cheque::CancelCheque()
{
    m_lAmount = 0;

    // When cancelling a cheque, it is basically just deposited back into the
    // account it was originally drawn from. The purpose of this is to "beat the
    // original recipient to the punch" by invalidating the cheque before he can
    // redeem it. Therefore when we do this "deposit" we don't actually intend
    // to
    // change the account balance -- so we set the cheque amount to 0.
    //
    // So why deposit the cheque, with a 0 balance? Because we just want to
    // invalidate the transaction number that was used on the cheque. We're
    // still
    // going to use a balance agreement, which the server will still verify, but
    // it
    // will be for a zero balance, and the transaction number will still be
    // marked
    // off via a cheque receipt.
    //
    // Since this is really just about marking off transaction numbers, not
    // changing any balances, we set the cheque amount to 0 and re-sign it.
}

// Imagine that you are actually writing a cheque.
// That's basically what this function does.
// Make sure to sign it afterwards.
auto Cheque::IssueCheque(
    const Amount& lAmount,
    const std::int64_t& lTransactionNum,
    const Time& VALID_FROM,
    const Time& VALID_TO,  // The expiration date (valid from/to dates) of
                           // the cheque
    const Identifier& SENDER_ACCT_ID,  // The asset account the cheque is drawn
                                       // on.
    const identifier::Nym& SENDER_NYM_ID,  // This ID must match the user ID on
                                           // the asset account,
    // AND must verify the cheque signature with that user's key.
    const String& strMemo,                             // Optional memo field.
    const identifier::Nym& pRECIPIENT_NYM_ID) -> bool  // Recipient optional.
                                                       // (Might be a blank
                                                       // cheque.)
{
    m_lAmount = lAmount;
    m_strMemo->Set(strMemo);

    SetValidFrom(VALID_FROM);
    SetValidTo(VALID_TO);

    SetTransactionNum(lTransactionNum);

    SetSenderAcctID(SENDER_ACCT_ID);
    SetSenderNymID(SENDER_NYM_ID);

    if (pRECIPIENT_NYM_ID.empty()) {
        m_bHasRecipient = false;
        m_RECIPIENT_NYM_ID->Release();
    } else {
        m_bHasRecipient = true;
        m_RECIPIENT_NYM_ID = pRECIPIENT_NYM_ID;
    }

    m_bHasRemitter = false;  // OTCheque::SetAsVoucher() will set this to true.

    if (m_lAmount < 0) m_strContractType->Set("INVOICE");

    return true;
}

void Cheque::InitCheque()
{
    m_strContractType->Set("CHEQUE");

    m_lAmount = 0;
    m_bHasRecipient = false;
    m_bHasRemitter = false;
}

void Cheque::Release_Cheque()
{
    // If there were any dynamically allocated objects, clean them up here.
    m_strMemo->Release();

    //    m_SENDER_ACCT_ID.Release();     // in parent class now.
    //    m_SENDER_NYM_ID.Release();     // in parent class now.
    m_RECIPIENT_NYM_ID->Release();

    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...

    // Then I call this to re-initialize everything
    InitCheque();
}

void Cheque::Release() { Release_Cheque(); }

Cheque::~Cheque() { Release_Cheque(); }
}  // namespace opentxs
