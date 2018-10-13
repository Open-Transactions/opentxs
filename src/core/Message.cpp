// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Message.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/consensus/Context.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/Proto.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::Message"

#define ERROR_STRING "error"
#define PING_NOTARY "pingNotary"
#define PING_NOTARY_RESPONSE "pingNotaryResponse"
#define REGISTER_NYM "registerNym"
#define REGISTER_NYM_RESPONSE "registerNymResponse"
#define UNREGISTER_NYM "unregisterNym"
#define UNREGISTER_NYM_RESPONSE "unregisterNymResponse"
#define GET_REQUEST_NUMBER "getRequestNumber"
#define GET_REQUEST_NUMBER_RESPONSE "getRequestNumberResponse"
#define GET_TRANSACTION_NUMBER "getTransactionNumbers"
#define GET_TRANSACTION_NUMBER_RESPONSE "getTransactionNumbersResponse"
#define CHECK_NYM "checkNym"
#define CHECK_NYM_RESPONSE "checkNymResponse"
#define SEND_NYM_MESSAGE "sendNymMessage"
#define SEND_NYM_MESSAGE_RESPONSE "sendNymMessageResponse"
#define SEND_NYM_INSTRUMENT "sendNymInstrument"
#define UNREGISTER_ACCOUNT "unregisterAccount"
#define UNREGISTER_ACCOUNT_RESPONSE "unregisterAccountResponse"
#define REGISTER_ACCOUNT "registerAccount"
#define REGISTER_ACCOUNT_RESPONSE "registerAccountResponse"
#define REGISTER_INSTRUMENT_DEFINITION "registerInstrumentDefinition"
#define REGISTER_INSTRUMENT_DEFINITION_RESPONSE                                \
    "registerInstrumentDefinitionResponse"
#define ISSUE_BASKET "issueBasket"
#define ISSUE_BASKET_RESPONSE "issueBasketResponse"
#define NOTARIZE_TRANSACTION "notarizeTransaction"
#define NOTARIZE_TRANSACTION_RESPONSE "notarizeTransactionResponse"
#define GET_NYMBOX "getNymbox"
#define GET_NYMBOX_RESPONSE "getNymboxResponse"
#define GET_BOX_RECEIPT "getBoxReceipt"
#define GET_BOX_RECEIPT_RESPONSE "getBoxReceiptResponse"
#define GET_ACCOUNT_DATA "getAccountData"
#define GET_ACCOUNT_DATA_RESPONSE "getAccountDataResponse"
#define PROCESS_NYMBOX "processNymbox"
#define PROCESS_NYMBOX_RESPONSE "processNymboxResponse"
#define PROCESS_INBOX "processInbox"
#define PROCESS_INBOX_RESPONSE "processInboxResponse"
#define QUERY_INSTRUMENT_DEFINITION "queryInstrumentDefinitions"
#define QUERY_INSTRUMENT_DEFINITION_RESPONSE                                   \
    "queryInstrumentDefinitionsResponse"
#define GET_INSTRUMENT_DEFINITION "getInstrumentDefinition"
#define GET_INSTRUMENT_DEFINITION_RESPONSE "getInstrumentDefinitionResponse"
#define GET_MINT "getMint"
#define GET_MINT_RESPONSE "getMintResponse"
#define GET_MARKET_LIST "getMarketList"
#define GET_MARKET_LIST_RESPONSE "getMarketListResponse"
#define GET_MARKET_OFFERS "getMarketOffers"
#define GET_MARKET_OFFERS_RESPONSE "getMarketOffersResponse"
#define GET_MARKET_RECENT_TRADES "getMarketRecentTrades"
#define GET_MARKET_RECENT_TRADES_RESPONSE "getMarketRecentTradesResponse"
#define GET_NYM_MARKET_OFFERS "getNymMarketOffers"
#define GET_NYM_MARKET_OFFERS_RESPONSE "getNymMarketOffersResponse"
#define TRIGGER_CLAUSE "triggerClause"
#define TRIGGER_CLAUSE_RESPONSE "triggerClauseResponse"
#define USAGE_CREDITS "usageCredits"
#define USAGE_CREDITS_RESPONSE "usageCreditsResponse"
#define REGISTER_CONTRACT "registerContract"
#define REGISTER_CONTRACT_RESPONSE "registerContractResponse"
#define REQUEST_ADMIN "requestAdmin"
#define REQUEST_ADMIN_RESPONSE "requestAdminResponse"
#define ADD_CLAIM "addClaim"
#define ADD_CLAIM_RESPONSE "addClaimResponse"

// PROTOCOL DOCUMENT

// --- This is the file that implements the entire message protocol.
// (Transactions are in a different file.)

// true  == success (even if nothing harvested.)
// false == error.
//

namespace opentxs
{

OTMessageStrategyManager Message::messageStrategyManager;

const Message::TypeMap Message::message_names_{
    {MessageType::badID, ERROR_STRING},
    {MessageType::pingNotary, PING_NOTARY},
    {MessageType::pingNotaryResponse, PING_NOTARY_RESPONSE},
    {MessageType::registerNym, REGISTER_NYM},
    {MessageType::registerNymResponse, REGISTER_NYM_RESPONSE},
    {MessageType::unregisterNym, UNREGISTER_NYM},
    {MessageType::unregisterNymResponse, UNREGISTER_NYM_RESPONSE},
    {MessageType::getRequestNumber, GET_REQUEST_NUMBER},
    {MessageType::getRequestNumberResponse, GET_REQUEST_NUMBER_RESPONSE},
    {MessageType::getTransactionNumbers, GET_TRANSACTION_NUMBER},
    {MessageType::getTransactionNumbersResponse,
     GET_TRANSACTION_NUMBER_RESPONSE},
    {MessageType::processNymbox, PROCESS_NYMBOX},
    {MessageType::processNymboxResponse, PROCESS_NYMBOX_RESPONSE},
    {MessageType::checkNym, CHECK_NYM},
    {MessageType::checkNymResponse, CHECK_NYM_RESPONSE},
    {MessageType::sendNymMessage, SEND_NYM_MESSAGE},
    {MessageType::sendNymMessageResponse, SEND_NYM_MESSAGE_RESPONSE},
    {MessageType::sendNymInstrument, SEND_NYM_INSTRUMENT},
    {MessageType::unregisterAccount, UNREGISTER_ACCOUNT},
    {MessageType::unregisterAccountResponse, UNREGISTER_ACCOUNT_RESPONSE},
    {MessageType::registerAccount, REGISTER_ACCOUNT},
    {MessageType::registerAccountResponse, REGISTER_ACCOUNT_RESPONSE},
    {MessageType::registerInstrumentDefinition, REGISTER_INSTRUMENT_DEFINITION},
    {MessageType::registerInstrumentDefinitionResponse,
     REGISTER_INSTRUMENT_DEFINITION_RESPONSE},
    {MessageType::issueBasket, ISSUE_BASKET},
    {MessageType::issueBasketResponse, ISSUE_BASKET_RESPONSE},
    {MessageType::notarizeTransaction, NOTARIZE_TRANSACTION},
    {MessageType::notarizeTransactionResponse, NOTARIZE_TRANSACTION_RESPONSE},
    {MessageType::getNymbox, GET_NYMBOX},
    {MessageType::getNymboxResponse, GET_NYMBOX_RESPONSE},
    {MessageType::getBoxReceipt, GET_BOX_RECEIPT},
    {MessageType::getBoxReceiptResponse, GET_BOX_RECEIPT_RESPONSE},
    {MessageType::getAccountData, GET_ACCOUNT_DATA},
    {MessageType::getAccountDataResponse, GET_ACCOUNT_DATA_RESPONSE},
    {MessageType::processNymbox, PROCESS_NYMBOX},
    {MessageType::processNymboxResponse, PROCESS_NYMBOX_RESPONSE},
    {MessageType::processInbox, PROCESS_INBOX},
    {MessageType::processInboxResponse, PROCESS_INBOX_RESPONSE},
    {MessageType::queryInstrumentDefinitions, QUERY_INSTRUMENT_DEFINITION},
    {MessageType::queryInstrumentDefinitionsResponse,
     QUERY_INSTRUMENT_DEFINITION_RESPONSE},
    {MessageType::getInstrumentDefinition, GET_INSTRUMENT_DEFINITION},
    {MessageType::getInstrumentDefinitionResponse,
     GET_INSTRUMENT_DEFINITION_RESPONSE},
    {MessageType::getMint, GET_MINT},
    {MessageType::getMintResponse, GET_MINT_RESPONSE},
    {MessageType::getMarketList, GET_MARKET_LIST},
    {MessageType::getMarketListResponse, GET_MARKET_LIST_RESPONSE},
    {MessageType::getMarketOffers, GET_MARKET_OFFERS},
    {MessageType::getMarketOffersResponse, GET_MARKET_OFFERS_RESPONSE},
    {MessageType::getMarketRecentTrades, GET_MARKET_RECENT_TRADES},
    {MessageType::getMarketRecentTradesResponse,
     GET_MARKET_RECENT_TRADES_RESPONSE},
    {MessageType::getNymMarketOffers, GET_NYM_MARKET_OFFERS},
    {MessageType::getNymMarketOffersResponse, GET_NYM_MARKET_OFFERS_RESPONSE},
    {MessageType::triggerClause, TRIGGER_CLAUSE},
    {MessageType::triggerClauseResponse, TRIGGER_CLAUSE_RESPONSE},
    {MessageType::usageCredits, USAGE_CREDITS},
    {MessageType::usageCreditsResponse, USAGE_CREDITS_RESPONSE},
    {MessageType::registerContract, REGISTER_CONTRACT},
    {MessageType::registerContractResponse, REGISTER_CONTRACT_RESPONSE},
    {MessageType::requestAdmin, REQUEST_ADMIN},
    {MessageType::requestAdminResponse, REQUEST_ADMIN_RESPONSE},
    {MessageType::addClaim, ADD_CLAIM},
    {MessageType::addClaimResponse, ADD_CLAIM_RESPONSE},
};

const std::map<MessageType, MessageType> Message::reply_message_{
    {MessageType::pingNotary, MessageType::pingNotaryResponse},
    {MessageType::registerNym, MessageType::registerNymResponse},
    {MessageType::unregisterNym, MessageType::unregisterNymResponse},
    {MessageType::getRequestNumber, MessageType::getRequestNumberResponse},
    {MessageType::getTransactionNumbers,
     MessageType::getTransactionNumbersResponse},
    {MessageType::checkNym, MessageType::checkNymResponse},
    {MessageType::sendNymMessage, MessageType::sendNymMessageResponse},
    {MessageType::unregisterAccount, MessageType::unregisterAccountResponse},
    {MessageType::registerAccount, MessageType::registerAccountResponse},
    {MessageType::registerInstrumentDefinition,
     MessageType::registerInstrumentDefinitionResponse},
    {MessageType::issueBasket, MessageType::issueBasketResponse},
    {MessageType::notarizeTransaction,
     MessageType::notarizeTransactionResponse},
    {MessageType::getNymbox, MessageType::getNymboxResponse},
    {MessageType::getBoxReceipt, MessageType::getBoxReceiptResponse},
    {MessageType::getAccountData, MessageType::getAccountDataResponse},
    {MessageType::processNymbox, MessageType::processNymboxResponse},
    {MessageType::processInbox, MessageType::processInboxResponse},
    {MessageType::queryInstrumentDefinitions,
     MessageType::queryInstrumentDefinitionsResponse},
    {MessageType::getInstrumentDefinition,
     MessageType::getInstrumentDefinitionResponse},
    {MessageType::getMint, MessageType::getMintResponse},
    {MessageType::getMarketList, MessageType::getMarketListResponse},
    {MessageType::getMarketOffers, MessageType::getMarketOffersResponse},
    {MessageType::getMarketRecentTrades,
     MessageType::getMarketRecentTradesResponse},
    {MessageType::getNymMarketOffers, MessageType::getNymMarketOffersResponse},
    {MessageType::triggerClause, MessageType::triggerClauseResponse},
    {MessageType::usageCredits, MessageType::usageCreditsResponse},
    {MessageType::registerContract, MessageType::registerContractResponse},
    {MessageType::requestAdmin, MessageType::requestAdminResponse},
    {MessageType::addClaim, MessageType::addClaimResponse},
};

const Message::ReverseTypeMap Message::message_types_ = make_reverse_map();

Message::Message(const api::Core& core)
    : Contract(core)
    , m_bIsSigned(false)
    , m_strCommand(String::Factory())
    , m_strNotaryID(String::Factory())
    , m_strNymID(String::Factory())
    , m_strNymboxHash(String::Factory())
    , m_strInboxHash(String::Factory())
    , m_strOutboxHash(String::Factory())
    , m_strNymID2(String::Factory())
    , m_strNymPublicKey(String::Factory())
    , m_strInstrumentDefinitionID(String::Factory())
    , m_strAcctID(String::Factory())
    , m_strType(String::Factory())
    , m_strRequestNum(String::Factory())
    , m_ascInReferenceTo(Armored::Factory())
    , m_ascPayload(Armored::Factory())
    , m_ascPayload2(Armored::Factory())
    , m_ascPayload3(Armored::Factory())
    , m_lNewRequestNum(0)
    , m_lDepth(0)
    , m_lTransactionNum(0)
    , m_bSuccess(false)
    , m_bBool(false)
    , m_lTime(0)
{
    Contract::m_strContractType->Set("MESSAGE");
}

Message::ReverseTypeMap Message::make_reverse_map()
{
    Message::ReverseTypeMap output{};

    for (const auto& it : message_names_) {
        const auto& type = it.first;
        const auto& name = it.second;
        output.emplace(name, type);
    }

    return output;
}

MessageType Message::reply_command(const MessageType& type)
{
    try {

        return reply_message_.at(type);
    } catch (const std::out_of_range&) {

        return MessageType::badID;
    }
}

std::string Message::Command(const MessageType type)
{
    try {

        return message_names_.at(type);
    } catch (const std::out_of_range&) {

        return ERROR_STRING;
    }
}

MessageType Message::Type(const std::string& type)
{
    try {

        return message_types_.at(type);
    } catch (const std::out_of_range&) {

        return MessageType::badID;
    }
}

std::string Message::ReplyCommand(const MessageType type)
{
    return Command(reply_command(type));
}

bool Message::HarvestTransactionNumbers(
    ServerContext& context,
    bool bHarvestingForRetry,           // false until positively asserted.
    bool bReplyWasSuccess,              // false until positively asserted.
    bool bReplyWasFailure,              // false until positively asserted.
    bool bTransactionWasSuccess,        // false until positively asserted.
    bool bTransactionWasFailure) const  // false until positively asserted.
{

    const auto MSG_NYM_ID = Identifier::Factory(m_strNymID),
               NOTARY_ID = Identifier::Factory(m_strNotaryID),
               ACCOUNT_ID = Identifier::Factory(
                   m_strAcctID->Exists() ? m_strAcctID
                                         : m_strNymID);  // This may be
    // unnecessary, but just
    // in case.

    const auto strLedger = String::Factory(m_ascPayload);
    auto theLedger = api_.Factory().Ledger(
        MSG_NYM_ID,
        ACCOUNT_ID,
        NOTARY_ID);  // We're going to
                     // load a messsage
                     // ledger from *this.

    if (!strLedger->Exists() || !theLedger->LoadLedgerFromString(strLedger)) {
        otErr << __FUNCTION__
              << ": ERROR: Failed trying to load message ledger:\n\n"
              << strLedger << "\n\n";
        return false;
    }

    // Let's iterate through the transactions inside, and harvest whatever
    // we can...
    for (auto& it : theLedger->GetTransactionMap()) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        // NOTE: You would ONLY harvest the transaction numbers if your
        // request failed.
        // Clearly you would never bother harvesting the numbers from a
        // SUCCESSFUL request,
        // because doing so would only put you out of sync. (This is the
        // same reason why
        // we DO harvest numbers from UNSUCCESSFUL requests--in order to
        // stay in sync.)
        //
        // That having been said, an important distinction must be made
        // between failed
        // requests where "the message succeeded but the TRANSACTION
        // failed", versus requests
        // where the MESSAGE ITSELF failed (meaning the transaction itself
        // never got a
        // chance to run, and thus never had a chance to fail.)
        //
        // In the first case, you don't want to harvest the opening
        // transaction number
        // (the primary transaction number for that transaction) because
        // that number was
        // already burned when the transaction failed. Instead, you want to
        // harvest "all
        // the others" (the "closing" numbers.)
        // But in the second case, you want to harvest the opening
        // transaction number as well,
        // since it is still good (because the transaction never ran.)
        //
        // (Therefore the below logic turns on whether or not the message
        // was a success.)
        //
        // UPDATE: The logic is now all inside
        // OTTransaction::Harvest...Numbers, you just have to tell it,
        // when you call it, the state of certain things (message success,
        // transaction success, etc.)
        //

        pTransaction->HarvestOpeningNumber(
            context,
            bHarvestingForRetry,
            bReplyWasSuccess,
            bReplyWasFailure,
            bTransactionWasSuccess,
            bTransactionWasFailure);

        // We grab the closing numbers no matter what (whether message
        // succeeded or failed.)
        // It bears mentioning one more time that you would NEVER harvest in
        // the first place unless
        // your original request somehow failed. So this is more about WHERE
        // the failure occurred (at
        // the message level or the transaction level), not WHETHER one
        // occurred.
        //
        pTransaction->HarvestClosingNumbers(
            context,
            bHarvestingForRetry,
            bReplyWasSuccess,
            bReplyWasFailure,
            bTransactionWasSuccess,
            bTransactionWasFailure);
    }

    return true;
}

// So the message can get the list of numbers from the Nym, before sending,
// that should be listed as acknowledged that the server reply has already been
// seen for those request numbers.
void Message::SetAcknowledgments(const Context& context)
{
    SetAcknowledgments(context.AcknowledgedNumbers());
}

void Message::SetAcknowledgments(const std::set<RequestNumber>& numbers)
{
    m_AcknowledgedReplies.Release();

    for (const auto& it : numbers) { m_AcknowledgedReplies.Add(it); }
}

// The framework (Contract) will call this function at the appropriate time.
// OTMessage is special because it actually does something here, when most
// contracts are read-only and thus never update their contents.
// Messages, obviously, are different every time, and this function will be
// called just prior to the signing of the message, in Contract::SignContract.
void Message::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    m_lTime = OTTimeGetCurrentTime();

    Tag tag("notaryMessage");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("dateSigned", formatTimestamp(m_lTime));

    if (!updateContentsByType(tag)) {
        TagPtr pTag(new Tag(m_strCommand->Get()));
        pTag->add_attribute("requestNum", m_strRequestNum->Get());
        pTag->add_attribute("success", formatBool(false));
        pTag->add_attribute("acctID", m_strAcctID->Get());
        pTag->add_attribute("nymID", m_strNymID->Get());
        pTag->add_attribute("notaryID", m_strNotaryID->Get());
        // The below was an XML comment in the previous version
        // of this code. It's unused.
        pTag->add_attribute("infoInvalid", "THIS IS AN INVALID MESSAGE");
        tag.add_tag(pTag);
    }

    // ACKNOWLEDGED REQUEST NUMBERS
    //
    // (For reducing the number of box receipts for replyNotices that
    // must be downloaded.)
    //
    // Client keeps a list of server replies he's already seen.
    // Server keeps a list of numbers the client has provided on HIS list
    // (server has removed those from Nymbox).
    //
    // (Each sends his respective list in every message.)
    //
    // Client removes any number he sees on the server's list.
    // Server removes any number he sees the client has also removed.
    //
    if (m_AcknowledgedReplies.Count() > 0) {
        auto strAck = String::Factory();
        if (m_AcknowledgedReplies.Output(strAck) && strAck->Exists()) {
            const auto ascTemp = Armored::Factory(strAck);
            if (ascTemp->Exists()) {
                tag.add_tag("ackReplies", ascTemp->Get());
            }
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

bool Message::updateContentsByType(Tag& parent)
{
    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(m_strCommand->Get());
    if (!strategy) return false;
    strategy->writeXml(*this, parent);
    return true;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Message::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    //
    // if (nReturnVal = Contract::ProcessXMLNode(xml))
    //      return nReturnVal;

    const auto strNodeName = String::Factory(xml->getNodeName());
    if (strNodeName->Compare("ackReplies")) {
        return processXmlNodeAckReplies(*this, xml);
    } else if (strNodeName->Compare("acknowledgedReplies")) {
        return processXmlNodeAcknowledgedReplies(*this, xml);
    } else if (strNodeName->Compare("notaryMessage")) {
        return processXmlNodeNotaryMessage(*this, xml);
    }

    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(xml->getNodeName());
    if (!strategy) return 0;
    return strategy->processXml(*this, xml);
}

std::int32_t Message::processXmlNodeAckReplies(
    __attribute__((unused)) Message& m,
    irr::io::IrrXMLReader*& xml)
{
    auto strDepth = String::Factory();
    if (!Contract::LoadEncodedTextField(xml, strDepth)) {
        otErr << "Error in OTMessage::ProcessXMLNode: ackReplies field "
                 "without value.\n";
        return (-1);  // error condition
    }

    m_AcknowledgedReplies.Release();

    if (strDepth->Exists()) m_AcknowledgedReplies.Add(strDepth);

    return 1;
}

std::int32_t Message::processXmlNodeAcknowledgedReplies(
    __attribute__((unused)) Message& m,
    irr::io::IrrXMLReader*& xml)
{
    otErr << "OTMessage::ProcessXMLNode: SKIPPING DEPRECATED FIELD: "
             "acknowledgedReplies\n";

    while (xml->getNodeType() != irr::io::EXN_ELEMENT_END) { xml->read(); }

    return 1;
}

std::int32_t Message::processXmlNodeNotaryMessage(
    __attribute__((unused)) Message& m,
    irr::io::IrrXMLReader*& xml)
{
    m_strVersion = String::Factory(xml->getAttributeValue("version"));

    auto strDateSigned = String::Factory(xml->getAttributeValue("dateSigned"));

    if (strDateSigned->Exists()) m_lTime = parseTimestamp(strDateSigned->Get());

    LogVerbose(OT_METHOD)(__FUNCTION__)(
        " ===> Loading XML for Message into memory structures... ")
        .Flush();

    return 1;
}

// OTString StrategyGetMarketListResponse::writeXml(OTMessage &message)

// std::int32_t StrategyGetMarketListResponse::processXml(OTMessage &message,
// irr::io::IrrXMLReader*& xml)

// Most contracts do not override this function...
// But OTMessage does, because every request sent to the server needs to be
// signed.
// And each new request is a new message, that requires a new signature, unlike
// most
// contracts, (that always stay the same after they are signed.)
//
// We need to update the m_xmlUnsigned member with the message members before
// the
// actual signing occurs. (Presumably this is the whole reason why the account
// is being re-signed.)
//
// Normally, in other Contract and derived classes, m_xmlUnsigned is read
// from the file and then kept read-only, since contracts do not normally
// change.
// But as new messages are sent, they must be signed. This function insures that
// the most up-to-date member contents are included in the request before it is
// signed.
//
// Note: Above comment is slightly old. This override is now here only for the
// purpose
// of releasing the signatures.  The other functionality is now handled by the
// UpdateContents member, which is called by the framework, and otherwise empty
// in
// default, but child classes such as OTMessage and OTAccount override it to
// save
// their contents just before signing.
// See OTMessage::UpdateContents near the top of this file for an example.
//
bool Message::SignContract(const Nym& theNym, const OTPasswordData* pPWData)
{
    // I release these, I assume, because a message only has one signer.
    ReleaseSignatures();  // Note: this might change with credentials. We might
                          // require multiple signatures.

    // Use the authentication key instead of the signing key.
    //
    m_bIsSigned = Contract::SignContractAuthent(theNym, pPWData);

    if (m_bIsSigned) {
        //        otErr <<
        // "\n******************************************************\n"
        //                "Contents of signed
        // message:\n\n%s******************************************************\n\n",
        // m_xmlUnsigned->Get());
    } else
        LogDetail(OT_METHOD)(__FUNCTION__)("Failure signing message: ")(
            m_xmlUnsigned)
            .Flush();

    return m_bIsSigned;
}

// virtual (Contract)
bool Message::VerifySignature(const Nym& theNym, const OTPasswordData* pPWData)
    const
{
    // Messages, unlike many contracts, use the authentication key instead of
    // the signing key. This is because signing keys are meant for signing
    // legally
    // binding agreements, whereas authentication keys are used for message
    // transport
    // and for file storage. Since this is OTMessage specifically, used for
    // transport,
    // we have overridden sign and verify contract methods, to explicitly use
    // the
    // authentication key instead of the signing key. OTSignedFile should
    // probably be
    // the same way. (Maybe it already is, by the time you are reading this.)
    //
    return VerifySigAuthent(theNym, pPWData);
}

// Unlike other contracts, which do not change over time, and thus calculate
// their ID
// from a hash of the file itself, OTMessage objects are different every time.
// Thus, we
// cannot use a hash of the file to produce the Message ID.
//
// Message ID will probably become an important part of the protocol (to prevent
// replay attacks..)
// So I will end up using it. But for now, VerifyContractID will always return
// true.
//
bool Message::VerifyContractID() const { return true; }

Message::~Message() {}

void OTMessageStrategy::processXmlSuccess(
    Message& m,
    irr::io::IrrXMLReader*& xml)
{
    m.m_bSuccess =
        String::Factory(xml->getAttributeValue("success"))->Compare("true");
}

void Message::registerStrategy(std::string name, OTMessageStrategy* strategy)
{
    messageStrategyManager.registerStrategy(name, strategy);
}

OTMessageStrategy::~OTMessageStrategy() {}

class StrategyGetMarketOffers : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("marketID", m.m_strNymID2->Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));

        parent.add_tag(pTag);
    }

    virtual std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) m.m_lDepth = strDepth->ToLong();

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\n Market ID: " << m.m_strNymID2
               << "\n Request #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffers::reg(
    "getMarketOffers",
    new StrategyGetMarketOffers());

class StrategyGetMarketOffersResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));
        pTag->add_attribute("marketID", m.m_strNymID2->Get());

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        } else if (!m.m_bSuccess && (m.m_ascInReferenceTo->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    virtual std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) m.m_lDepth = strDepth->ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyGetMarketOffersResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload->Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILED")(" NymID:    ")(
                m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)(" MarketID: ")(
                m.m_strNymID2)
                .Flush();  // m_ascPayload.Get()
        else
            LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILED")(" NymID:    ")(
                m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)(" MarketID: ")(
                m.m_strNymID2)
                .Flush();  // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffersResponse::reg(
    "getMarketOffersResponse",
    new StrategyGetMarketOffersResponse());

class StrategyGetMarketRecentTrades : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("marketID", m.m_strNymID2->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
            " NymID:    ")(m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)(
            " Market ID: ")(m.m_strNymID2)(" Request #: ")(m.m_strRequestNum)
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTrades::reg(
    "getMarketRecentTrades",
    new StrategyGetMarketRecentTrades());

class StrategyGetMarketRecentTradesResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));
        pTag->add_attribute("marketID", m.m_strNymID2->Get());

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        } else if (!m.m_bSuccess && (m.m_ascInReferenceTo->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    virtual std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) m.m_lDepth = strDepth->ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload->Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILED")(" NymID:    ")(
                m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)(" MarketID: ")(
                m.m_strNymID2)
                .Flush();  // m_ascPayload.Get()
        else
            LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILED")(" NymID:    ")(
                m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)(" MarketID: ")(
                m.m_strNymID2)
                .Flush();  // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTradesResponse::reg(
    "getMarketRecentTradesResponse",
    new StrategyGetMarketRecentTradesResponse());

class StrategyGetNymMarketOffers : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    virtual std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
            " NymID:    ")(m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)(
            " Request #: ")(m.m_strRequestNum)
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffers::reg(
    "getNymMarketOffers",
    new StrategyGetNymMarketOffers());

class StrategyGetNymMarketOffersResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        } else if (!m.m_bSuccess && (m.m_ascInReferenceTo->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    virtual std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) m.m_lDepth = strDepth->ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload->Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILED")(" NymID:    ")(
                m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)
                .Flush();  // m_ascPayload.Get()
        else
            LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILED")(" NymID:    ")(
                m.m_strNymID)("\n NotaryID: ")(m.m_strNotaryID)
                .Flush();  // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffersResponse::reg(
    "getNymMarketOffersResponse",
    new StrategyGetNymMarketOffersResponse());

class StrategyPingNotary : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        TagPtr pAuthentKeyTag(
            new Tag("publicAuthentKey", m.m_strNymPublicKey->Get()));
        TagPtr pEncryptKeyTag(
            new Tag("publicEncryptionKey", m.m_strNymID2->Get()));

        pAuthentKeyTag->add_attribute(
            "type",
            crypto::key::Asymmetric::KeyTypeToString(
                static_cast<proto::AsymmetricKeyType>(m.keytypeAuthent_))
                ->Get());
        pEncryptKeyTag->add_attribute(
            "type",
            crypto::key::Asymmetric::KeyTypeToString(
                static_cast<proto::AsymmetricKeyType>(m.keytypeEncrypt_))
                ->Get());

        pTag->add_tag(pAuthentKeyTag);
        pTag->add_tag(pEncryptKeyTag);

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        // -------------------------------------------------
        const char* pElementExpected = "publicAuthentKey";
        auto ascTextExpected = Armored::Factory();

        String::Map temp_MapAttributesAuthent;
        temp_MapAttributesAuthent.insert(std::pair<std::string, std::string>(
            "type",
            ""));  // Value should be "RSA" after reading.
        // -----------------------------------------------
        if (!Contract::LoadEncodedTextFieldByName(
                xml,
                ascTextExpected,
                pElementExpected,
                &temp_MapAttributesAuthent)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }
        {
            // -----------------------------------------------
            auto it = temp_MapAttributesAuthent.find("type");

            if ((it != temp_MapAttributesAuthent.end()))  // We expected this
                                                          // much.
            {
                std::string& str_type = it->second;

                if (str_type.size() > 0)  // Success finding key type.
                {
                    m.keytypeAuthent_ =
                        crypto::key::Asymmetric::StringToKeyType(
                            String::Factory(str_type));
                }
            }
            // -----------------------------------------------
        }
        m.m_strNymPublicKey->Set(ascTextExpected);
        // -------------------------------------------------
        pElementExpected = "publicEncryptionKey";
        ascTextExpected->Release();

        String::Map temp_MapAttributesEncrypt;
        temp_MapAttributesEncrypt.insert(std::pair<std::string, std::string>(
            "type",
            ""));  // Value should be "RSA" after reading.
        // -----------------------------------------------
        if (!Contract::LoadEncodedTextFieldByName(
                xml,
                ascTextExpected,
                pElementExpected,
                &temp_MapAttributesEncrypt)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }
        {
            // -----------------------------------------------
            auto it = temp_MapAttributesEncrypt.find("type");

            if ((it != temp_MapAttributesEncrypt.end()))  // We expected this
                                                          // much.
            {
                std::string& str_type = it->second;

                if (str_type.size() > 0)  // Success finding key type.
                {
                    m.keytypeEncrypt_ =
                        crypto::key::Asymmetric::StringToKeyType(
                            String::Factory(str_type));
                }
            }
        }
        // -----------------------------------------------
        m.m_strNymID2->Set(ascTextExpected);
        // -------------------------------------------------
        LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
            " NymID:    ")(m.m_strNymID)(" NotaryID: ")(m.m_strNotaryID)(
            " Public signing key: ")(m.m_strNymPublicKey)(
            " Public encryption key: ")(m.m_strNymID2)
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotary::reg(
    "pingNotary",
    new StrategyPingNotary());

class StrategyPingNotaryResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        LogDetail(OT_METHOD)(__FUNCTION__)(" Command: ")(m.m_strCommand)(
            " Success: ")(m.m_bSuccess ? "true" : "false")("\nNymID:    ")(
            m.m_strNymID)("NotaryID: ")(m.m_strNotaryID)
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotaryResponse::reg(
    "pingNotaryResponse",
    new StrategyPingNotaryResponse());

class StrategyRegisterContract : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("contract", m.m_ascPayload->Get());
        pTag->add_attribute("type", std::to_string(m.enum_));

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_ascPayload->Set(xml->getAttributeValue("contract"));

        try {
            m.enum_ = std::stoi(xml->getAttributeValue("type"));
        } catch (const std::invalid_argument&) {
            m.enum_ = 0;
        } catch (const std::out_of_range&) {
            m.enum_ = 0;
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterContract::reg(
    "registerContract",
    new StrategyRegisterContract());

class StrategyRegisterContractResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in StrategyRegisterContractResponse: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterContractResponse::reg(
    "registerContractResponse",
    new StrategyRegisterContractResponse());

class StrategyRegisterNym : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("publicnym", m.m_ascPayload->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_ascPayload->Set(xml->getAttributeValue("publicnym"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNym::reg(
    "registerNym",
    new StrategyRegisterNym());

class StrategyRegisterNymResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2)) {
            pTag->add_tag("nymfile", m.m_ascPayload->Get());
        }

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        if (m.m_bSuccess) {
            const char* pElementExpected = "nymfile";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNymResponse::reg(
    "registerNymResponse",
    new StrategyRegisterNymResponse());

class StrategyUnregisterNym : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNym::reg(
    "unregisterNym",
    new StrategyUnregisterNym());

class StrategyUnregisterNymResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNymResponse::reg(
    "unregisterNymResponse",
    new StrategyUnregisterNymResponse());

class StrategyCheckNym : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNym::reg("checkNym", new StrategyCheckNym());

class StrategyCheckNymResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        // This means new-style credentials are being sent, not just the public
        // key as before.
        const bool bCredentials = (m.m_ascPayload->Exists());
        OT_ASSERT(!m.m_bSuccess || bCredentials);

        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_bSuccess && bCredentials) {
            pTag->add_tag("publicnym", m.m_ascPayload->Get());
        } else {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        auto ascTextExpected = Armored::Factory();
        const char* pElementExpected = nullptr;

        if (!m.m_bSuccess) {
            pElementExpected = "inReferenceTo";
            m.m_ascInReferenceTo = ascTextExpected;
            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        } else {  // Success.
            pElementExpected = "publicnym";
            ascTextExpected->Release();

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
            m.m_ascPayload = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\nNymID2:    " << m.m_strNymID2
                   << "\n"
                      "NotaryID: "
                   << m.m_strNotaryID << "\nNym2 Public Key:\n"
                   << m.m_strNymPublicKey << "\n\n";
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\nNymID2:    " << m.m_strNymID2
                   << "\n"
                      "NotaryID: "
                   << m.m_strNotaryID << "\n\n";  // m.m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNymResponse::reg(
    "checkNymResponse",
    new StrategyCheckNymResponse());

class StrategyUsageCredits : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("adjustment", formatLong(m.m_lDepth));

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        auto strAdjustment =
            String::Factory(xml->getAttributeValue("adjustment"));

        if (strAdjustment->GetLength() > 0)
            m.m_lDepth = strAdjustment->ToLong();

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum
               << "\nAdjustment: " << m.m_lDepth << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCredits::reg(
    "usageCredits",
    new StrategyUsageCredits());

class StrategyUsageCreditsResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("totalCredits", formatLong(m.m_lDepth));

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        auto strTotalCredits =
            String::Factory(xml->getAttributeValue("totalCredits"));

        if (strTotalCredits->GetLength() > 0)
            m.m_lDepth = strTotalCredits->ToLong();

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\nTotal Credits: " << m.m_lDepth
               << " \n\n";
        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCreditsResponse::reg(
    "usageCreditsResponse",
    new StrategyUsageCreditsResponse());

// This one isn't part of the message protocol, but is used for
// outmail storage.
// (Because outmail isn't encrypted like the inmail is, since the
// Nymfile itself will soon be encrypted, and there's no need to
// be redundant also as well in addition on top of that.
//
class StrategyOutpaymentsMessageOrOutmailMessage : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascPayload->GetLength() > 2) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg(
    "outpaymentsMessage",
    new StrategyOutpaymentsMessageOrOutmailMessage());
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg2(
    "outmailMessage",
    new StrategyOutpaymentsMessageOrOutmailMessage());

class StrategySendNymMessage : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascPayload->GetLength() > 2) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessage::reg(
    "sendNymMessage",
    new StrategySendNymMessage());

class StrategySendNymMessageResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessageResponse::reg(
    "sendNymMessageResponse",
    new StrategySendNymMessageResponse());

// sendNymInstrument is sent from one user
// to the server, which then attaches that
// message as a payment, onto a transaction
// on the Nymbox of the recipient.
//
// payDividend is not a normal user
// message. Rather, the sender uses
// notarizeTransaction to do a
// payDividend transaction. On the
// server side, this creates a new
// message of type "payDividend"
// for each recipient, in order to
// attach a voucher to it (for each
// recipient) and then that
// (artificially created
// payDividend msg) is added to the
// Nymbox of each recipient.
class StrategySendNymInstrumentOrPayDividend : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascPayload->GetLength() > 2) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg(
    "sendNymInstrument",
    new StrategySendNymInstrumentOrPayDividend());
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg2(
    "payDividend",
    new StrategySendNymInstrumentOrPayDividend());

class StrategyGetRequestNumber : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestNumber::reg(
    "getRequestNumber",
    new StrategyGetRequestNumber());

// This is the ONE command where you see a request number coming
// back from the server.
// In all the other commands, it should be SENT to the server, not
// received from the server.
class StrategyGetRequestResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("newRequestNum", formatLong(m.m_lNewRequestNum));
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const auto strNewRequestNum =
            String::Factory(xml->getAttributeValue("newRequestNum"));
        m.m_lNewRequestNum =
            strNewRequestNum->Exists() ? strNewRequestNum->ToLong() : 0;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID
               << "\nRequest Number:    " << m.m_strRequestNum
               << "  New Number: " << m.m_lNewRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestResponse::reg(
    "getRequestNumberResponse",
    new StrategyGetRequestResponse());

class StrategyRegisterInstrumentDefinition : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());

        if (m.m_ascPayload->GetLength()) {
            pTag->add_tag("instrumentDefinition", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "instrumentDefinition";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\nRequest#: " << m.m_strRequestNum
               << "\nAsset Type:\n"
               << m.m_strInstrumentDefinitionID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinition::reg(
    "registerInstrumentDefinition",
    new StrategyRegisterInstrumentDefinition());

class StrategyRegisterInstrumentDefinitionResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());
        // the new issuer account ID
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("issuerAccount", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr
                    << "Error in StrategyRegisterInstrumentDefinitionResponse: "
                       "Expected "
                    << pElementExpected << " element with text field, for "
                    << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "issuerAccount";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr
                    << "Error in StrategyRegisterInstrumentDefinitionResponse: "
                       "Expected "
                    << pElementExpected << " element with text field, for "
                    << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload->GetLength())) {
            otErr << "Error in StrategyRegisterInstrumentDefinitionResponse:\n"
                     "Expected issuerAccount and/or inReferenceTo elements "
                     "with text fields in "
                     "registerInstrumentDefinitionResponse reply\n";
            return (-1);  // error condition
        }

        auto acctContents = String::Factory(m.m_ascPayload);
        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\nInstrument Definition ID: "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinitionResponse::reg(
    "registerInstrumentDefinitionResponse",
    new StrategyRegisterInstrumentDefinitionResponse());

class StrategyQueryInstrumentDefinitions : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascPayload->GetLength()) {
            pTag->add_tag("stringMap", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "stringMap";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\nRequest#: " << m.m_strRequestNum
               << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitions::reg(
    "queryInstrumentDefinitions",
    new StrategyQueryInstrumentDefinitions());

class StrategyQueryInstrumentDefinitionsResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("stringMap", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyQueryInstrumentDefinitionsResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "stringMap";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyQueryInstrumentDefinitionsResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload->GetLength())) {
            otErr << "Error in StrategyQueryInstrumentDefinitionsResponse:\n"
                     "Expected stringMap and/or inReferenceTo elements with "
                     "text fields in "
                     "queryInstrumentDefinitionsResponse reply\n";
            return (-1);  // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitionsResponse::reg(
    "queryInstrumentDefinitionsResponse",
    new StrategyQueryInstrumentDefinitionsResponse());

class StrategyIssueBasket : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascPayload->GetLength()) {
            pTag->add_tag("currencyBasket", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "currencyBasket";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the Payload isn't there, then failure.
        if (!m.m_ascPayload->GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected currencyBasket element with text fields in "
                     "issueBasket message\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\nRequest#: " << m.m_strRequestNum
               << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasket::reg(
    "issueBasket",
    new StrategyIssueBasket());

class StrategyIssueBasketResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected inReferenceTo element with text fields in "
                     "issueBasketResponse reply\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\nInstrumentDefinitionID: " << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasketResponse::reg(
    "issueBasketResponse",
    new StrategyIssueBasketResponse());

class StrategyRegisterAccount : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\nRequest#: " << m.m_strRequestNum
               << "\nAsset Type:\n"
               << m.m_strInstrumentDefinitionID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccount::reg(
    "registerAccount",
    new StrategyRegisterAccount());

class StrategyRegisterAccountResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascInReferenceTo->Exists()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->Exists()) {
            pTag->add_tag("newAccount", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                //                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "newAccount";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        //
        if (m.m_bSuccess && !m.m_ascPayload->GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected newAccount element with text field, in "
                     "registerAccountResponse reply\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccountResponse::reg(
    "registerAccountResponse",
    new StrategyRegisterAccountResponse());

class StrategyGetBoxReceipt : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        // If retrieving box receipt for Nymbox, NymID
        // will appear in this variable.
        pTag->add_attribute("accountID", m.m_strAcctID->Get());
        pTag->add_attribute(
            "boxType",  // outbox is 2.
            (m.m_lDepth == 0) ? "nymbox"
                              : ((m.m_lDepth == 1) ? "inbox" : "outbox"));
        pTag->add_attribute("transactionNum", formatLong(m.m_lTransactionNum));

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        auto strTransactionNum =
            String::Factory(xml->getAttributeValue("transactionNum"));
        m.m_lTransactionNum =
            strTransactionNum->Exists() ? strTransactionNum->ToLong() : 0;

        const auto strBoxType =
            String::Factory(xml->getAttributeValue("boxType"));

        if (strBoxType->Compare("nymbox"))
            m.m_lDepth = 0;
        else if (strBoxType->Compare("inbox"))
            m.m_lDepth = 1;
        else if (strBoxType->Compare("outbox"))
            m.m_lDepth = 2;
        else {
            m.m_lDepth = 0;
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxType to be inbox, outbox, or nymbox, in "
                     "getBoxReceipt\n";
            return (-1);
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n Request#: " << m.m_strRequestNum
               << "  Transaction#: " << m.m_lTransactionNum << "   boxType: "
               << ((m.m_lDepth == 0) ? "nymbox"
                                     : (m.m_lDepth == 1) ? "inbox" : "outbox")
               << "\n\n";  // outbox is 2.);

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceipt::reg(
    "getBoxReceipt",
    new StrategyGetBoxReceipt());

class StrategyGetBoxReceiptResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());
        pTag->add_attribute(
            "boxType",  // outbox is 2.
            (m.m_lDepth == 0) ? "nymbox"
                              : ((m.m_lDepth == 1) ? "inbox" : "outbox"));
        pTag->add_attribute("transactionNum", formatLong(m.m_lTransactionNum));

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("boxReceipt", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));

        auto strTransactionNum =
            String::Factory(xml->getAttributeValue("transactionNum"));
        m.m_lTransactionNum =
            strTransactionNum->Exists() ? strTransactionNum->ToLong() : 0;

        const auto strBoxType =
            String::Factory(xml->getAttributeValue("boxType"));

        if (strBoxType->Compare("nymbox"))
            m.m_lDepth = 0;
        else if (strBoxType->Compare("inbox"))
            m.m_lDepth = 1;
        else if (strBoxType->Compare("outbox"))
            m.m_lDepth = 2;
        else {
            m.m_lDepth = 0;
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxType to be inbox, outbox, or nymbox, in "
                     "getBoxReceiptResponse reply\n";
            return (-1);
        }

        // inReferenceTo contains the getBoxReceipt (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "boxReceipt";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload->GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxReceipt and/or inReferenceTo elements with "
                     "text fields in "
                     "getBoxReceiptResponse reply\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceiptResponse::reg(
    "getBoxReceiptResponse",
    new StrategyGetBoxReceiptResponse());

class StrategyUnregisterAccount : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n Request#: " << m.m_strRequestNum
               << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccount::reg(
    "unregisterAccount",
    new StrategyUnregisterAccount());

class StrategyUnregisterAccountResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));

        // inReferenceTo contains the unregisterAccount (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyUnregisterAccount: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength()) {
            otErr << "Error in StrategyUnregisterAccount:\n"
                     "Expected inReferenceTo element with text fields in "
                     "unregisterAccountResponse reply\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccountResponse::reg(
    "unregisterAccountResponse",
    new StrategyUnregisterAccountResponse());

class StrategyNotarizeTransaction : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascPayload->GetLength()) {
            pTag->add_tag("accountLedger", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "accountLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n Request#: " << m.m_strRequestNum
               << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransaction::reg(
    "notarizeTransaction",
    new StrategyNotarizeTransaction());

class StrategyNotarizeTransactionResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }
        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("responseLedger", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.
        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyNotarizeTransactionResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }
        if (m.m_bSuccess) {  // Successful message (should contain
                             // responseLedger).
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyNotarizeTransactionResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (!m.m_ascPayload->GetLength() && m.m_bSuccess)) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "notarizeTransactionResponse reply\n";
            return (-1);  // error condition
        }

        //      OTString acctContents(m.m_ascPayload);
        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n AccountID: " << m.m_strAcctID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransactionResponse::reg(
    "notarizeTransactionResponse",
    new StrategyNotarizeTransactionResponse());

class StrategyGetTransactionNumbers : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n Request#: " << m.m_strRequestNum
               << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbers::reg(
    "getTransactionNumbers",
    new StrategyGetTransactionNumbers());

class StrategyGetTransactionNumbersResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbersResponse::reg(
    "getTransactionNumbersResponse",
    new StrategyGetTransactionNumbersResponse());

class StrategyGetNymbox : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymbox::reg("getNymbox", new StrategyGetNymbox());

class StrategyGetNymboxResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("nymboxLedger", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "nymboxLedger";
        else
            pElementExpected = "inReferenceTo";

        auto ascTextExpected = Armored::Factory();

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymboxResponse::reg(
    "getNymboxResponse",
    new StrategyGetNymboxResponse());

class StrategyGetAccountData : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAccountID:    " << m.m_strAcctID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountData::reg(
    "getAccountData",
    new StrategyGetAccountData());

class StrategyGetAccountDataResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());
        pTag->add_attribute("inboxHash", m.m_strInboxHash->Get());
        pTag->add_attribute("outboxHash", m.m_strOutboxHash->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess) {
            if (m.m_ascPayload->GetLength()) {
                pTag->add_tag("account", m.m_ascPayload->Get());
            }
            if (m.m_ascPayload2->GetLength()) {
                pTag->add_tag("inbox", m.m_ascPayload2->Get());
            }
            if (m.m_ascPayload3->GetLength()) {
                pTag->add_tag("outbox", m.m_ascPayload3->Get());
            }
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strInboxHash = String::Factory(xml->getAttributeValue("inboxHash"));
        m.m_strOutboxHash =
            String::Factory(xml->getAttributeValue("outboxHash"));

        if (m.m_bSuccess) {
            if (!Contract::LoadEncodedTextFieldByName(
                    xml, m.m_ascPayload, "account")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected account "
                         "element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, m.m_ascPayload2, "inbox")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected inbox"
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1);  // error condition
            }

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, m.m_ascPayload3, "outbox")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected outbox"
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1);  // error condition
            }
        } else {  // Message success=false
            if (!Contract::LoadEncodedTextFieldByName(
                    xml, m.m_ascInReferenceTo, "inReferenceTo")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected "
                         "inReferenceTo element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID:    " << m.m_strAcctID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountDataResponse::reg(
    "getAccountDataResponse",
    new StrategyGetAccountDataResponse());

class StrategyGetInstrumentDefinition : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAsset Type:    " << m.m_strInstrumentDefinitionID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinition::reg(
    "getInstrumentDefinition",
    new StrategyGetInstrumentDefinition());

class StrategyGetInstrumentDefinitionResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("instrumentDefinition", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "instrumentDefinition";
        else
            pElementExpected = "inReferenceTo";

        auto ascTextExpected = Armored::Factory();

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nInstrument Definition ID:    "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinitionResponse::reg(
    "getInstrumentDefinitionResponse",
    new StrategyGetInstrumentDefinitionResponse());

class StrategyGetMint : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAsset Type:    " << m.m_strInstrumentDefinitionID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMint::reg("getMint", new StrategyGetMint());

class StrategyGetMintResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("mint", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "mint";
        else
            pElementExpected = "inReferenceTo";

        auto ascTextExpected = Armored::Factory();

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in StrategyGetMintResponse: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nInstrument Definition ID:    "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: "
               << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMintResponse::reg(
    "getMintResponse",
    new StrategyGetMintResponse());

class StrategyProcessInbox : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascPayload->GetLength()) {
            pTag->add_tag("processLedger", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "processLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n Request#: " << m.m_strRequestNum
               << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInbox::reg(
    "processInbox",
    new StrategyProcessInbox());

class StrategyProcessInboxResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }
        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("responseLedger", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.
        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyProcessInboxResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {  // Success.
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyProcessInboxResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (!m.m_ascPayload->GetLength() && m.m_bSuccess)) {
            otErr << "Error in StrategyProcessInboxResponse:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "processInboxResponse reply\n";
            return (-1);  // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n AccountID: " << m.m_strAcctID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInboxResponse::reg(
    "processInboxResponse",
    new StrategyProcessInboxResponse());

class StrategyProcessNymbox : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        if (m.m_ascPayload->GetLength()) {
            pTag->add_tag("processLedger", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "processLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n Request#: " << m.m_strRequestNum
               << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymbox::reg(
    "processNymbox",
    new StrategyProcessNymbox());

class StrategyProcessNymboxResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }
        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("responseLedger", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.
        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyProcessNymboxResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {  // Success
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in StrategyProcessNymboxResponse: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (!m.m_ascPayload->GetLength() && m.m_bSuccess)) {
            otErr << "Error in StrategyProcessNymboxResponse:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "processNymboxResponse reply\n";
            return (-1);  // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: "
               << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymboxResponse::reg(
    "processNymboxResponse",
    new StrategyProcessNymboxResponse());

class StrategyTriggerClause : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("smartContractID", formatLong(m.m_lTransactionNum));
        pTag->add_attribute("clauseName", m.m_strNymID2->Get());
        pTag->add_attribute("hasParam", formatBool(m.m_ascPayload->Exists()));

        if (m.m_ascPayload->Exists()) {
            pTag->add_tag("parameter", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("clauseName"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        const auto strHasParam =
            String::Factory(xml->getAttributeValue("hasParam"));

        auto strTransactionNum =
            String::Factory(xml->getAttributeValue("smartContractID"));
        if (strTransactionNum->Exists())
            m.m_lTransactionNum = strTransactionNum->ToLong();

        if (strHasParam->Compare("true")) {
            const char* pElementExpected = "parameter";
            auto ascTextExpected = Armored::Factory();

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            } else
                m.m_ascPayload = ascTextExpected;
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nClause TransNum and Name:  " << m.m_lTransactionNum
               << "  /  " << m.m_strNymID2
               << " \n"
                  "Request #: "
               << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClause::reg(
    "triggerClause",
    new StrategyTriggerClause());

class StrategyTriggerClauseResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";

        auto ascTextExpected = Armored::Factory();

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in StrategyTriggerClauseResponse: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "   NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClauseResponse::reg(
    "triggerClauseResponse",
    new StrategyTriggerClauseResponse());

class StrategyGetMarketList : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    virtual std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketList::reg(
    "getMarketList",
    new StrategyGetMarketList());

class StrategyGetMarketListResponse : public OTMessageStrategy
{
public:
    virtual std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) m.m_lDepth = strDepth->ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!Contract::LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected "
                      << pElementExpected << " element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1);  // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload->Set(ascTextExpected);
            else
                m.m_ascInReferenceTo->Set(ascTextExpected);
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n";  // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n";  // m_ascInReferenceTo.Get()

        return 1;
    }

    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        } else if (!m.m_bSuccess && (m.m_ascInReferenceTo->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketListResponse::reg(
    "getMarketListResponse",
    new StrategyGetMarketListResponse());

class StrategyRequestAdmin : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("password", m.m_strAcctID->Get());

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID->Set(xml->getAttributeValue("password"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};

RegisterStrategy StrategyRequestAdmin::reg(
    "requestAdmin",
    new StrategyRequestAdmin());

class StrategyRequestAdminResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in StrategyRequestAdminResponse: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRequestAdminResponse::reg(
    "requestAdminResponse",
    new StrategyRequestAdminResponse());

class StrategyAddClaim : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("section", m.m_strNymID2->Get());
        pTag->add_attribute("type", m.m_strInstrumentDefinitionID->Get());
        pTag->add_attribute("value", m.m_strAcctID->Get());
        pTag->add_attribute("primary", formatBool(m.m_bBool));

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymID2->Set(xml->getAttributeValue("section"));
        m.m_strInstrumentDefinitionID->Set(xml->getAttributeValue("type"));
        m.m_strAcctID->Set(xml->getAttributeValue("value"));
        const auto primary = String::Factory(xml->getAttributeValue("primary"));
        m.m_bBool = primary->Compare("true");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};

RegisterStrategy StrategyAddClaim::reg("addClaim", new StrategyAddClaim());

class StrategyAddClaimResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    std::int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            otErr << "Error in StrategyAddClaimResponse: "
                     "Expected "
                  << pElementExpected << " element with text field, for "
                  << m.m_strCommand << ".\n";
            return (-1);  // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyAddClaimResponse::reg(
    "addClaimResponse",
    new StrategyAddClaimResponse());
}  // namespace opentxs
