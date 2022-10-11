// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "internal/otx/common/Cheque.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstring>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
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
    , amount_(0)
    , memo_(String::Factory())
    , recipient_nym_id_()
    , has_recipient_(false)
    , remitter_nym_id_()
    , remitter_account_id_()
    , has_remitter_(false)
{
    InitCheque();
}

Cheque::Cheque(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    : ot_super(api, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , amount_(0)
    , memo_(String::Factory())
    , recipient_nym_id_()
    , has_recipient_(false)
    , remitter_nym_id_()
    , remitter_account_id_()
    , has_remitter_(false)
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
    xml_unsigned_->Release();

    Tag tag("cheque");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("amount", [&] {
        auto buf = UnallocatedCString{};
        amount_.Serialize(writer(buf));
        return buf;
    }());
    tag.add_attribute(
        "instrumentDefinitionID", INSTRUMENT_DEFINITION_ID->Get());
    tag.add_attribute("transactionNum", std::to_string(GetTransactionNum()));
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute("senderAcctID", SENDER_ACCT_ID->Get());
    tag.add_attribute("senderNymID", SENDER_NYM_ID->Get());
    tag.add_attribute("hasRecipient", formatBool(has_recipient_));
    tag.add_attribute(
        "recipientNymID", has_recipient_ ? RECIPIENT_NYM_ID->Get() : "");
    tag.add_attribute("hasRemitter", formatBool(has_remitter_));
    tag.add_attribute(
        "remitterNymID", has_remitter_ ? REMITTER_NYM_ID->Get() : "");
    tag.add_attribute(
        "remitterAcctID", has_remitter_ ? REMITTER_ACCT_ID->Get() : "");

    tag.add_attribute("validFrom", from);
    tag.add_attribute("validTo", to);

    if (memo_->Exists() && memo_->GetLength() > 2) {
        auto ascMemo = Armored::Factory(memo_);
        tag.add_tag("memo", ascMemo->Get());
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
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
        has_recipient_ = strHasRecipient->Compare("true");

        auto strHasRemitter =
            String::Factory(xml->getAttributeValue("hasRemitter"));
        has_remitter_ = strHasRemitter->Compare("true");

        version_ = String::Factory(xml->getAttributeValue("version"));
        amount_ = factory::Amount(xml->getAttributeValue("amount"));

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
            api_.Factory().UnitIDFromBase58(strInstrumentDefinitionID->Bytes());
        const auto NOTARY_ID =
            api_.Factory().NotaryIDFromBase58(strNotaryID->Bytes());
        const auto SENDER_ACCT_ID =
            api_.Factory().IdentifierFromBase58(strSenderAcctID->Bytes());
        const auto SENDER_NYM_ID =
            api_.Factory().NymIDFromBase58(strSenderNymID->Bytes());

        SetInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
        SetNotaryID(NOTARY_ID);
        SetSenderAcctID(SENDER_ACCT_ID);
        SetSenderNymID(SENDER_NYM_ID);

        // Recipient ID
        if (has_recipient_) {
            recipient_nym_id_ =
                api_.Factory().NymIDFromBase58(strRecipientNymID->Bytes());
        } else {
            recipient_nym_id_.clear();
        }

        // Remitter ID (for vouchers)
        if (has_remitter_) {
            remitter_nym_id_ =
                api_.Factory().NymIDFromBase58(strRemitterNymID->Bytes());
            remitter_account_id_ =
                api_.Factory().IdentifierFromBase58(strRemitterAcctID->Bytes());
        } else {
            remitter_nym_id_.clear();
            remitter_account_id_.clear();
        }
        {
            const auto unittype =
                api_.Wallet().Internal().CurrencyTypeBasedOnUnitType(
                    INSTRUMENT_DEFINITION_ID);
            LogVerbose()(OT_PRETTY_CLASS())("Cheque Amount: ")(
                amount_, unittype)(". Transaction Number: ")(
                transaction_num_)(" Valid From: ")(
                str_valid_from)(" Valid To: ")(
                str_valid_to)(" InstrumentDefinitionID: ")(
                strInstrumentDefinitionID.get())(" NotaryID: ")(
                strNotaryID.get())(" senderAcctID: ")(strSenderAcctID.get())(
                " senderNymID: ")(strSenderNymID.get())(" Has Recipient? ")(
                has_recipient_ ? "Yes" : "No")(
                ". If yes, NymID of Recipient: ")(strRecipientNymID.get())(
                " Has Remitter? ")(has_remitter_ ? "Yes" : "No")(
                ". If yes, NymID/Acct of Remitter: ")(strRemitterNymID.get())(
                " / ")(strRemitterAcctID.get())
                .Flush();
        }
        nReturnVal = 1;
    } else if (!strcmp("memo", xml->getNodeName())) {
        if (!LoadEncodedTextField(xml, memo_)) {
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
    amount_ = 0;

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
    const identifier::Generic& SENDER_ACCT_ID,  // The asset account the cheque
                                                // is drawn on.
    const identifier::Nym& SENDER_NYM_ID,  // This ID must match the user ID on
                                           // the asset account,
    // AND must verify the cheque signature with that user's key.
    const String& strMemo,                             // Optional memo field.
    const identifier::Nym& pRECIPIENT_NYM_ID) -> bool  // Recipient optional.
                                                       // (Might be a blank
                                                       // cheque.)
{
    amount_ = lAmount;
    memo_->Set(strMemo);

    SetValidFrom(VALID_FROM);
    SetValidTo(VALID_TO);

    SetTransactionNum(lTransactionNum);

    SetSenderAcctID(SENDER_ACCT_ID);
    SetSenderNymID(SENDER_NYM_ID);

    if (pRECIPIENT_NYM_ID.empty()) {
        has_recipient_ = false;
        recipient_nym_id_.clear();
    } else {
        has_recipient_ = true;
        recipient_nym_id_ = pRECIPIENT_NYM_ID;
    }

    has_remitter_ = false;  // OTCheque::SetAsVoucher() will set this to true.

    if (amount_ < 0) { contract_type_->Set("INVOICE"); }

    return true;
}

void Cheque::InitCheque()
{
    contract_type_->Set("CHEQUE");

    amount_ = 0;
    has_recipient_ = false;
    has_remitter_ = false;
}

void Cheque::Release_Cheque()
{
    // If there were any dynamically allocated objects, clean them up here.
    memo_->Release();

    //    sender_account_id_.Release();     // in parent class now.
    //    sender_nym_id_.Release();     // in parent class now.
    recipient_nym_id_.clear();

    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...

    // Then I call this to re-initialize everything
    InitCheque();
}

void Cheque::Release() { Release_Cheque(); }

Cheque::~Cheque() { Release_Cheque(); }
}  // namespace opentxs
