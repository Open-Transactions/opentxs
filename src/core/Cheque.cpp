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

#include "opentxs/stdafx.hpp"

#include "opentxs/core/Cheque.hpp"

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <cstring>
#include <ostream>
#include <string>

using namespace irr;
using namespace io;

#define OT_METHOD "opentxs::Cheque::"

namespace opentxs
{
Cheque::Cheque()
    : ot_super()
    , m_lAmount(0)
    , m_strMemo()
    , m_RECIPIENT_NYM_ID(Identifier::Factory())
    , m_bHasRecipient(false)
    , m_REMITTER_NYM_ID(Identifier::Factory())
    , m_REMITTER_ACCT_ID(Identifier::Factory())
    , m_bHasRemitter(false)
{
    InitCheque();
}

Cheque::Cheque(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID)
    : ot_super(NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lAmount(0)
    , m_strMemo()
    , m_RECIPIENT_NYM_ID(Identifier::Factory())
    , m_bHasRecipient(false)
    , m_REMITTER_NYM_ID(Identifier::Factory())
    , m_REMITTER_ACCT_ID(Identifier::Factory())
    , m_bHasRemitter(false)
{
    InitCheque();
}

std::unique_ptr<Cheque> Cheque::CreateFromReceipt(const OTTransaction& receipt)
{
    std::unique_ptr<Cheque> output{new Cheque};

    OT_ASSERT(output)

    String serializedItem{};
    receipt.GetReferenceString(serializedItem);
    std::unique_ptr<Item> item(Item::CreateItemFromString(
        serializedItem,
        receipt.GetRealNotaryID(),
        receipt.GetReferenceToNum()));

    OT_ASSERT(item)

    String serializedCheque{};
    item->GetAttachment(serializedCheque);
    const auto loaded = output->LoadContractFromString(serializedCheque);

    if (false == loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load cheque,"
              << std::endl;
    }

    return output;
}

void Cheque::UpdateContents()
{
    String INSTRUMENT_DEFINITION_ID(GetInstrumentDefinitionID()),
        NOTARY_ID(GetNotaryID()), SENDER_ACCT_ID(GetSenderAcctID()),
        SENDER_NYM_ID(GetSenderNymID()), RECIPIENT_NYM_ID(GetRecipientNymID()),
        REMITTER_NYM_ID(GetRemitterNymID()),
        REMITTER_ACCT_ID(GetRemitterAcctID());

    std::string from = formatTimestamp(GetValidFrom());
    std::string to = formatTimestamp(GetValidTo());

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag tag("cheque");

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("amount", formatLong(m_lAmount));
    tag.add_attribute("instrumentDefinitionID", INSTRUMENT_DEFINITION_ID.Get());
    tag.add_attribute("transactionNum", formatLong(GetTransactionNum()));
    tag.add_attribute("notaryID", NOTARY_ID.Get());
    tag.add_attribute("senderAcctID", SENDER_ACCT_ID.Get());
    tag.add_attribute("senderNymID", SENDER_NYM_ID.Get());
    tag.add_attribute("hasRecipient", formatBool(m_bHasRecipient));
    tag.add_attribute(
        "recipientNymID", m_bHasRecipient ? RECIPIENT_NYM_ID.Get() : "");
    tag.add_attribute("hasRemitter", formatBool(m_bHasRemitter));
    tag.add_attribute(
        "remitterNymID", m_bHasRemitter ? REMITTER_NYM_ID.Get() : "");
    tag.add_attribute(
        "remitterAcctID", m_bHasRemitter ? REMITTER_ACCT_ID.Get() : "");

    tag.add_attribute("validFrom", from);
    tag.add_attribute("validTo", to);

    if (m_strMemo.Exists() && m_strMemo.GetLength() > 2) {
        OTASCIIArmor ascMemo(m_strMemo);
        tag.add_tag("memo", ascMemo.Get());
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Cheque::ProcessXMLNode(IrrXMLReader*& xml)
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
        String strHasRecipient = xml->getAttributeValue("hasRecipient");
        m_bHasRecipient = strHasRecipient.Compare("true");

        String strHasRemitter = xml->getAttributeValue("hasRemitter");
        m_bHasRemitter = strHasRemitter.Compare("true");

        m_strVersion = xml->getAttributeValue("version");
        m_lAmount = String::StringToLong(xml->getAttributeValue("amount"));

        SetTransactionNum(
            String::StringToLong(xml->getAttributeValue("transactionNum")));

        const std::string str_valid_from = xml->getAttributeValue("validFrom");
        const std::string str_valid_to = xml->getAttributeValue("validTo");

        SetValidFrom(parseTimestamp(str_valid_from));
        SetValidTo(parseTimestamp(str_valid_to));

        String strInstrumentDefinitionID(
            xml->getAttributeValue("instrumentDefinitionID")),
            strNotaryID(xml->getAttributeValue("notaryID")),
            strSenderAcctID(xml->getAttributeValue("senderAcctID")),
            strSenderNymID(xml->getAttributeValue("senderNymID")),
            strRecipientNymID(xml->getAttributeValue("recipientNymID")),
            strRemitterNymID(xml->getAttributeValue("remitterNymID")),
            strRemitterAcctID(xml->getAttributeValue("remitterAcctID"));

        auto INSTRUMENT_DEFINITION_ID =
                 Identifier::Factory(strInstrumentDefinitionID),
             NOTARY_ID = Identifier::Factory(strNotaryID),
             SENDER_ACCT_ID = Identifier::Factory(strSenderAcctID),
             SENDER_NYM_ID = Identifier::Factory(strSenderNymID);

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

        otInfo << "\n\nCheque Amount: " << m_lAmount
               << ".  Transaction Number: " << m_lTransactionNum
               << "\n Valid From: " << str_valid_from
               << "\n Valid To: " << str_valid_to
               << "\n InstrumentDefinitionID: " << strInstrumentDefinitionID
               << "\n NotaryID: " << strNotaryID
               << "\n"
                  " senderAcctID: "
               << strSenderAcctID << "\n senderNymID: " << strSenderNymID
               << "\n "
                  " Has Recipient? "
               << (m_bHasRecipient ? "Yes" : "No")
               << ". If yes, NymID of Recipient: " << strRecipientNymID
               << "\n"
                  " Has Remitter? "
               << (m_bHasRemitter ? "Yes" : "No")
               << ". If yes, NymID/Acct of Remitter: " << strRemitterNymID
               << " / " << strRemitterAcctID << "\n";

        nReturnVal = 1;
    } else if (!strcmp("memo", xml->getNodeName())) {
        if (!Contract::LoadEncodedTextField(xml, m_strMemo)) {
            otErr << "Error in OTCheque::ProcessXMLNode: memo field without "
                     "value.\n";
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
bool Cheque::IssueCheque(
    const std::int64_t& lAmount,
    const std::int64_t& lTransactionNum,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO,  // The expiration date (valid from/to dates) of
                               // the cheque
    const Identifier& SENDER_ACCT_ID,  // The asset account the cheque is drawn
                                       // on.
    const Identifier& SENDER_NYM_ID,   // This ID must match the user ID on the
                                       // asset account,
    // AND must verify the cheque signature with that user's key.
    const String& strMemo,                // Optional memo field.
    const Identifier& pRECIPIENT_NYM_ID)  // Recipient optional.
                                          // (Might be a blank
                                          // cheque.)
{
    m_lAmount = lAmount;
    m_strMemo = strMemo;

    SetValidFrom(VALID_FROM);
    SetValidTo(VALID_TO);

    SetTransactionNum(lTransactionNum);

    SetSenderAcctID(SENDER_ACCT_ID);
    SetSenderNymID(SENDER_NYM_ID);

    if (pRECIPIENT_NYM_ID->empty()) {
        m_bHasRecipient = false;
        m_RECIPIENT_NYM_ID->Release();
    } else {
        m_bHasRecipient = true;
        m_RECIPIENT_NYM_ID = pRECIPIENT_NYM_ID;
    }

    m_bHasRemitter = false;  // OTCheque::SetAsVoucher() will set this to true.

    if (m_lAmount < 0) m_strContractType.Set("INVOICE");

    return true;
}

void Cheque::InitCheque()
{
    m_strContractType.Set("CHEQUE");

    m_lAmount = 0;
    m_bHasRecipient = false;
    m_bHasRemitter = false;
}

void Cheque::Release_Cheque()
{
    // If there were any dynamically allocated objects, clean them up here.
    m_strMemo.Release();

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
