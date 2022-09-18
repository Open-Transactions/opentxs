// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "internal/otx/common/Message.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/otx/consensus/Base.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

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
#define OUTMAIL "outmailMessage"

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
    {MessageType::outmail, OUTMAIL},
};

const UnallocatedMap<MessageType, MessageType> Message::reply_message_{
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

Message::Message(const api::Session& api)
    : Contract(api)
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
    , m_AcknowledgedReplies()
    , m_lNewRequestNum(0)
    , m_lDepth(0)
    , m_lTransactionNum(0)
    , m_bSuccess(false)
    , m_bBool(false)
    , m_lTime(0)
{
    Contract::m_strContractType->Set("MESSAGE");
}

auto Message::make_reverse_map() -> Message::ReverseTypeMap
{
    Message::ReverseTypeMap output{};

    for (const auto& it : message_names_) {
        const auto& type = it.first;
        const auto& name = it.second;
        output.emplace(name, type);
    }

    return output;
}

auto Message::reply_command(const MessageType& type) -> MessageType
{
    try {

        return reply_message_.at(type);
    } catch (...) {

        return MessageType::badID;
    }
}

auto Message::Command(const MessageType type) -> UnallocatedCString
{
    try {

        return message_names_.at(type);
    } catch (...) {

        return ERROR_STRING;
    }
}

auto Message::Type(const UnallocatedCString& type) -> MessageType
{
    try {

        return message_types_.at(type);
    } catch (...) {

        return MessageType::badID;
    }
}

auto Message::ReplyCommand(const MessageType type) -> UnallocatedCString
{
    return Command(reply_command(type));
}

auto Message::HarvestTransactionNumbers(
    otx::context::Server& context,
    bool bHarvestingForRetry,     // false until positively asserted.
    bool bReplyWasSuccess,        // false until positively asserted.
    bool bReplyWasFailure,        // false until positively asserted.
    bool bTransactionWasSuccess,  // false until positively asserted.
    bool bTransactionWasFailure) const
    -> bool  // false until positively asserted.
{

    const auto MSG_NYM_ID = api_.Factory().NymIDFromBase58(m_strNymID->Bytes());
    const auto NOTARY_ID =
        api_.Factory().NotaryIDFromBase58(m_strNotaryID->Bytes());
    const auto ACCOUNT_ID = api_.Factory().IdentifierFromBase58(
        (m_strAcctID->Exists() ? m_strAcctID : m_strNymID)->Bytes());  // This
                                                                       // may be
    // unnecessary, but just
    // in case.

    const auto strLedger = String::Factory(m_ascPayload);
    auto theLedger = api_.Factory().InternalSession().Ledger(
        MSG_NYM_ID,
        ACCOUNT_ID,
        NOTARY_ID);  // We're going to
                     // load a messsage
                     // ledger from *this.

    if (!strLedger->Exists() || !theLedger->LoadLedgerFromString(strLedger)) {
        LogError()(OT_PRETTY_CLASS())(
            "ERROR: Failed trying to load message ledger: ")(strLedger.get())(
            ".")
            .Flush();
        return false;
    }

    // Let's iterate through the transactions inside, and harvest whatever
    // we can...
    for (const auto& it : theLedger->GetTransactionMap()) {
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
void Message::SetAcknowledgments(const otx::context::Base& context)
{
    SetAcknowledgments(context.AcknowledgedNumbers());
}

void Message::SetAcknowledgments(const UnallocatedSet<RequestNumber>& numbers)
{
    m_AcknowledgedReplies.Release();

    for (const auto& it : numbers) { m_AcknowledgedReplies.Add(it); }
}

// The framework (Contract) will call this function at the appropriate time.
// OTMessage is special because it actually does something here, when most
// contracts are read-only and thus never update their contents.
// Messages, obviously, are different every time, and this function will be
// called just prior to the signing of the message, in Contract::SignContract.
void Message::UpdateContents(const PasswordPrompt& reason)
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    m_lTime = Clock::to_time_t(Clock::now());

    Tag tag("notaryMessage");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("dateSigned", formatTimestamp(convert_stime(m_lTime)));

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

    UnallocatedCString str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate(String::Factory(str_result));
}

auto Message::updateContentsByType(Tag& parent) -> bool
{
    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(m_strCommand->Get());
    if (!strategy) { return false; }
    strategy->writeXml(*this, parent);
    return true;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto Message::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
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
    if (!strategy) { return 0; }
    return strategy->processXml(*this, xml);
}

auto Message::processXmlNodeAckReplies(
    [[maybe_unused]] Message& m,
    irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    auto strDepth = String::Factory();
    if (!LoadEncodedTextField(xml, strDepth)) {
        LogError()(OT_PRETTY_CLASS())("Error: ackReplies field "
                                      "without value.")
            .Flush();
        return (-1);  // error condition
    }

    m_AcknowledgedReplies.Release();

    if (strDepth->Exists()) { m_AcknowledgedReplies.Add(strDepth); }

    return 1;
}

auto Message::processXmlNodeAcknowledgedReplies(
    [[maybe_unused]] Message& m,
    irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    LogError()(OT_PRETTY_CLASS())("SKIPPING DEPRECATED FIELD: "
                                  "acknowledgedReplies.")
        .Flush();

    while (xml->getNodeType() != irr::io::EXN_ELEMENT_END) { xml->read(); }

    return 1;
}

auto Message::processXmlNodeNotaryMessage(
    [[maybe_unused]] Message& m,
    irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    m_strVersion = String::Factory(xml->getAttributeValue("version"));

    auto strDateSigned = String::Factory(xml->getAttributeValue("dateSigned"));

    if (strDateSigned->Exists()) {
        m_lTime = Clock::to_time_t(parseTimestamp(strDateSigned->Get()));
    }

    LogVerbose()(OT_PRETTY_CLASS())(
        "===> Loading XML for Message into memory structures... ")
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
auto Message::SignContract(
    const identity::Nym& theNym,
    const PasswordPrompt& reason) -> bool
{
    // I release these, I assume, because a message only has one signer.
    ReleaseSignatures();  // Note: this might change with credentials. We might
                          // require multiple signatures.

    // Use the authentication key instead of the signing key.
    //
    m_bIsSigned = Contract::SignContractAuthent(theNym, reason);

    if (false == m_bIsSigned) {
        LogError()(OT_PRETTY_CLASS())("Failure signing message: ")(
            m_xmlUnsigned.get())
            .Flush();
    }

    return m_bIsSigned;
}

// (Contract)
auto Message::VerifySignature(const identity::Nym& theNym) const -> bool
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
    return VerifySigAuthent(theNym);
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
auto Message::VerifyContractID() const -> bool { return true; }

Message::~Message() = default;

void OTMessageStrategy::processXmlSuccess(
    Message& m,
    irr::io::IrrXMLReader*& xml)
{
    m.m_bSuccess =
        String::Factory(xml->getAttributeValue("success"))->Compare("true");
}

void Message::registerStrategy(
    UnallocatedCString name,
    OTMessageStrategy* strategy)
{
    messageStrategyManager.registerStrategy(name, strategy);
}

OTMessageStrategy::~OTMessageStrategy() = default;

class StrategyGetMarketOffers final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("marketID", m.m_strNymID2->Get());
        pTag->add_attribute("depth", std::to_string(m.m_lDepth));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.m_lDepth = strDepth->ToLong(); }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Market ID: ")(m.m_strNymID2.get())(
            " Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffers::reg(
    GET_MARKET_OFFERS,
    new StrategyGetMarketOffers());

class StrategyGetMarketOffersResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", std::to_string(m.m_lDepth));
        pTag->add_attribute("marketID", m.m_strNymID2->Get());

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        } else if (!m.m_bSuccess && (m.m_ascInReferenceTo->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.m_lDepth = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.m_bSuccess) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.m_bSuccess) {
                m.m_ascPayload->Set(ascTextExpected);
            } else {
                m.m_ascInReferenceTo = ascTextExpected;
            }
        }

        if (m.m_bSuccess) {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
                " MarketID: ")(m.m_strNymID2.get())
                .Flush();  // m_ascPayload.Get()
        } else {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
                " MarketID: ")(m.m_strNymID2.get())
                .Flush();  // m_ascInReferenceTo.Get()
        }

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffersResponse::reg(
    GET_MARKET_OFFERS_RESPONSE,
    new StrategyGetMarketOffersResponse());

class StrategyGetMarketRecentTrades final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("marketID", m.m_strNymID2->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Market ID: ")(m.m_strNymID2.get())(
            " Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTrades::reg(
    GET_MARKET_RECENT_TRADES,
    new StrategyGetMarketRecentTrades());

class StrategyGetMarketRecentTradesResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", std::to_string(m.m_lDepth));
        pTag->add_attribute("marketID", m.m_strNymID2->Get());

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        } else if (!m.m_bSuccess && (m.m_ascInReferenceTo->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.m_lDepth = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.m_bSuccess) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.m_bSuccess) {
                m.m_ascPayload->Set(ascTextExpected);
            } else {
                m.m_ascInReferenceTo = ascTextExpected;
            }
        }

        if (m.m_bSuccess) {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
                " MarketID: ")(m.m_strNymID2.get())
                .Flush();  // m_ascPayload.Get()
        } else {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
                " MarketID: ")(m.m_strNymID2.get())
                .Flush();  // m_ascInReferenceTo.Get()
        }

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTradesResponse::reg(
    GET_MARKET_RECENT_TRADES_RESPONSE,
    new StrategyGetMarketRecentTradesResponse());

class StrategyGetNymMarketOffers final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffers::reg(
    GET_NYM_MARKET_OFFERS,
    new StrategyGetNymMarketOffers());

class StrategyGetNymMarketOffersResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", std::to_string(m.m_lDepth));

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        } else if (!m.m_bSuccess && (m.m_ascInReferenceTo->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.m_lDepth = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.m_bSuccess) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.m_bSuccess) {
                m.m_ascPayload->Set(ascTextExpected);
            } else {
                m.m_ascInReferenceTo = ascTextExpected;
            }
        }

        if (m.m_bSuccess) {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
                .Flush();  // m_ascPayload.Get()
        } else {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
                .Flush();  // m_ascInReferenceTo.Get()
        }

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffersResponse::reg(
    GET_NYM_MARKET_OFFERS_RESPONSE,
    new StrategyGetNymMarketOffersResponse());

class StrategyPingNotary final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        TagPtr pAuthentKeyTag(
            new Tag("publicAuthentKey", m.m_strNymPublicKey->Get()));
        TagPtr pEncryptKeyTag(
            new Tag("publicEncryptionKey", m.m_strNymID2->Get()));

        pAuthentKeyTag->add_attribute("type", "notused");
        pEncryptKeyTag->add_attribute("type", "notused");

        pTag->add_tag(pAuthentKeyTag);
        pTag->add_tag(pEncryptKeyTag);

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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
        temp_MapAttributesAuthent.insert(
            std::pair<UnallocatedCString, UnallocatedCString>(
                "type",
                ""));  // Value should be "RSA" after reading.
        // -----------------------------------------------
        if (!LoadEncodedTextFieldByName(
                xml,
                ascTextExpected,
                pElementExpected,
                &temp_MapAttributesAuthent)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        m.m_strNymPublicKey->Set(ascTextExpected);

        pElementExpected = "publicEncryptionKey";
        ascTextExpected->Release();

        String::Map temp_MapAttributesEncrypt;
        temp_MapAttributesEncrypt.insert(
            std::pair<UnallocatedCString, UnallocatedCString>(
                "type",
                ""));  // Value should be "RSA" after reading.
        // -----------------------------------------------
        if (!LoadEncodedTextFieldByName(
                xml,
                ascTextExpected,
                pElementExpected,
                &temp_MapAttributesEncrypt)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        m.m_strNymID2->Set(ascTextExpected);

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Public signing key: ")(
            m.m_strNymPublicKey.get())(" Public encryption key: ")(
            m.m_strNymID2.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotary::reg(PING_NOTARY, new StrategyPingNotary());

class StrategyPingNotaryResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " Success: ")(m.m_bSuccess ? "true" : "false")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotaryResponse::reg(
    PING_NOTARY_RESPONSE,
    new StrategyPingNotaryResponse());

class StrategyRegisterContract final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("contract", m.m_ascPayload->Get());
        pTag->add_attribute("type", std::to_string(m.enum_));
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_ascPayload->Set(xml->getAttributeValue("contract"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        try {
            m.enum_ = static_cast<std::uint8_t>(
                std::stoi(xml->getAttributeValue("type")));
        } catch (...) {
            m.enum_ = 0;
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterContract::reg(
    REGISTER_CONTRACT,
    new StrategyRegisterContract());

class StrategyRegisterContractResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())("  ")(
            m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterContractResponse::reg(
    REGISTER_CONTRACT_RESPONSE,
    new StrategyRegisterContractResponse());

class StrategyRegisterNym final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("publicnym", m.m_ascPayload->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_ascPayload->Set(xml->getAttributeValue("publicnym"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNym::reg(
    REGISTER_NYM,
    new StrategyRegisterNym());

class StrategyRegisterNymResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        if (m.m_bSuccess && (m.m_ascPayload->GetLength() > 2)) {
            pTag->add_tag("nymfile", m.m_ascPayload->Get());
        }

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        if (m.m_bSuccess) {
            const char* pElementExpected = "nymfile";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())("  ")(
            m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNymResponse::reg(
    REGISTER_NYM_RESPONSE,
    new StrategyRegisterNymResponse());

class StrategyUnregisterNym final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNym::reg(
    UNREGISTER_NYM,
    new StrategyUnregisterNym());

class StrategyUnregisterNymResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())("  ")(
            m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNymResponse::reg(
    UNREGISTER_NYM_RESPONSE,
    new StrategyUnregisterNymResponse());

class StrategyCheckNym final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NymID2:    ")(
            m.m_strNymID2.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNym::reg(CHECK_NYM, new StrategyCheckNym());

class StrategyCheckNymResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        // This means new-style credentials are being sent, not just the public
        // key as before.
        const bool bCredentials = (m.m_ascPayload->Exists());
        OT_ASSERT(!m.m_bBool || bCredentials);

        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("found", formatBool(m.m_bBool));

        if (m.m_bBool && bCredentials) {
            pTag->add_tag("publicnym", m.m_ascPayload->Get());
        }

        if (false == m.m_bSuccess) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto found = String::Factory(xml->getAttributeValue("found"));
        m.m_bBool = found->Compare("true");

        auto ascTextExpected = Armored::Factory();
        const char* pElementExpected = nullptr;

        if (!m.m_bSuccess) {
            pElementExpected = "inReferenceTo";
            m.m_ascInReferenceTo = ascTextExpected;
            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        } else if (m.m_bBool) {  // Success.
            pElementExpected = "publicnym";
            ascTextExpected->Release();

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
            m.m_ascPayload = ascTextExpected;
        }

        if (m.m_bBool) {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NymID2:    ")(m.m_strNymID2.get())(
                " NotaryID: ")(m.m_strNotaryID.get())(" Nym2 Public Key: ")(
                m.m_strNymPublicKey.get())
                .Flush();
        } else {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NymID2:    ")(m.m_strNymID2.get())(
                " NotaryID: ")(m.m_strNotaryID.get())
                .Flush();
        }
        // m.m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNymResponse::reg(
    CHECK_NYM_RESPONSE,
    new StrategyCheckNymResponse());

class StrategyUsageCredits final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("adjustment", std::to_string(m.m_lDepth));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        auto strAdjustment =
            String::Factory(xml->getAttributeValue("adjustment"));

        if (strAdjustment->GetLength() > 0) {
            m.m_lDepth = strAdjustment->ToLong();
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NymID2:    ")(
            m.m_strNymID2.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request #: ")(m.m_strRequestNum.get())(" Adjustment: ")(
            m.m_lDepth)
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCredits::reg(
    USAGE_CREDITS,
    new StrategyUsageCredits());

class StrategyUsageCreditsResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("totalCredits", std::to_string(m.m_lDepth));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

        if (strTotalCredits->GetLength() > 0) {
            m.m_lDepth = strTotalCredits->ToLong();
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NymID2:    ")(m.m_strNymID2.get())(
            " NotaryID: ")(m.m_strNotaryID.get())(" Total Credits: ")(
            m.m_lDepth)
            .Flush();
        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCreditsResponse::reg(
    USAGE_CREDITS_RESPONSE,
    new StrategyUsageCreditsResponse());

// This one isn't part of the message protocol, but is used for
// outmail storage.
// (Because outmail isn't encrypted like the inmail is, since the
// Nymfile itself will soon be encrypted, and there's no need to
// be redundant also as well in addition on top of that.
//
class StrategyOutpaymentsMessageOrOutmailMessage final
    : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NymID2:    ")(
            m.m_strNymID2.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg(
    "outpaymentsMessage",
    new StrategyOutpaymentsMessageOrOutmailMessage());
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg2(
    OUTMAIL,
    new StrategyOutpaymentsMessageOrOutmailMessage());

class StrategySendNymMessage final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        if (m.m_ascPayload->GetLength() > 2) {
            pTag->add_tag("messagePayload", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NymID2:    ")(
            m.m_strNymID2.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessage::reg(
    SEND_NYM_MESSAGE,
    new StrategySendNymMessage());

class StrategySendNymMessageResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("nymID2", m.m_strNymID2->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NymID2:    ")(m.m_strNymID2.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessageResponse::reg(
    SEND_NYM_MESSAGE_RESPONSE,
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
class StrategySendNymInstrumentOrPayDividend final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymID2 = String::Factory(xml->getAttributeValue("nymID2"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NymID2:    ")(
            m.m_strNymID2.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg(
    SEND_NYM_INSTRUMENT,
    new StrategySendNymInstrumentOrPayDividend());
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg2(
    "payDividend",
    new StrategySendNymInstrumentOrPayDividend());

class StrategyGetRequestNumber final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestNumber::reg(
    GET_REQUEST_NUMBER,
    new StrategyGetRequestNumber());

// This is the ONE command where you see a request number coming
// back from the server.
// In all the other commands, it should be SENT to the server, not
// received from the server.
class StrategyGetRequestResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "newRequestNum", std::to_string(m.m_lNewRequestNum));
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request Number:    ")(m.m_strRequestNum.get())(" New Number: ")(
            m.m_lNewRequestNum)
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestResponse::reg(
    GET_REQUEST_NUMBER_RESPONSE,
    new StrategyGetRequestResponse());

class StrategyRegisterInstrumentDefinition final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request#: ")(m.m_strRequestNum.get())(
            " Asset Type: ")(m.m_strInstrumentDefinitionID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinition::reg(
    REGISTER_INSTRUMENT_DEFINITION,
    new StrategyRegisterInstrumentDefinition());

class StrategyRegisterInstrumentDefinitionResponse final
    : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "issuerAccount";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload->GetLength())) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: "
                "Expected issuerAccount and/or inReferenceTo elements "
                "with text fields in "
                "registerInstrumentDefinitionResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        auto acctContents = String::Factory(m.m_ascPayload);
        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID: ")(m.m_strAcctID.get())(
            " Instrument Definition ID: ")(m.m_strInstrumentDefinitionID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinitionResponse::reg(
    REGISTER_INSTRUMENT_DEFINITION_RESPONSE,
    new StrategyRegisterInstrumentDefinitionResponse());

class StrategyQueryInstrumentDefinitions final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "stringMap";
        Armored& ascTextExpected = m.m_ascPayload;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request#: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitions::reg(
    QUERY_INSTRUMENT_DEFINITION,
    new StrategyQueryInstrumentDefinitions());

class StrategyQueryInstrumentDefinitionsResponse final
    : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "stringMap";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload->GetLength())) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected stringMap and/or inReferenceTo elements "
                "with text fields in "
                "queryInstrumentDefinitionsResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitionsResponse::reg(
    QUERY_INSTRUMENT_DEFINITION_RESPONSE,
    new StrategyQueryInstrumentDefinitionsResponse());

class StrategyIssueBasket final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "currencyBasket";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the Payload isn't there, then failure.
        if (!m.m_ascPayload->GetLength()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected currencyBasket element with text fields in "
                "issueBasket message.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request#: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasket::reg(
    ISSUE_BASKET,
    new StrategyIssueBasket());

class StrategyIssueBasketResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected inReferenceTo element with text fields in "
                "issueBasketResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID: ")(m.m_strAcctID.get())(
            " InstrumentDefinitionID: ")(m.m_strInstrumentDefinitionID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasketResponse::reg(
    ISSUE_BASKET_RESPONSE,
    new StrategyIssueBasketResponse());

class StrategyRegisterAccount final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request#: ")(m.m_strRequestNum.get())(
            " Asset Type: ")(m.m_strInstrumentDefinitionID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccount::reg(
    REGISTER_ACCOUNT,
    new StrategyRegisterAccount());

class StrategyRegisterAccountResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                //                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "newAccount";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        //
        if (m.m_bSuccess && !m.m_ascPayload->GetLength()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected newAccount element with text field, in "
                "registerAccountResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID: ")(m.m_strAcctID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccountResponse::reg(
    REGISTER_ACCOUNT_RESPONSE,
    new StrategyRegisterAccountResponse());

class StrategyGetBoxReceipt final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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
        pTag->add_attribute(
            "transactionNum", std::to_string(m.m_lTransactionNum));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

        if (strBoxType->Compare("nymbox")) {
            m.m_lDepth = 0;
        } else if (strBoxType->Compare("inbox")) {
            m.m_lDepth = 1;
        } else if (strBoxType->Compare("outbox")) {
            m.m_lDepth = 2;
        } else {
            m.m_lDepth = 0;
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected boxType to be inbox, outbox, or nymbox, in "
                "getBoxReceipt.")
                .Flush();
            return (-1);
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" AccountID:    ")(
            m.m_strAcctID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request#: ")(m.m_strRequestNum.get())(" Transaction#: ")(
            m.m_lTransactionNum)(" boxType: ")(((m.m_lDepth == 0)   ? "nymbox"
                                                : (m.m_lDepth == 1) ? "inbox"
                                                                    : "outbox"))
            .Flush();  // outbox is 2.);

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceipt::reg(
    GET_BOX_RECEIPT,
    new StrategyGetBoxReceipt());

class StrategyGetBoxReceiptResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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
        pTag->add_attribute(
            "transactionNum", std::to_string(m.m_lTransactionNum));

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("boxReceipt", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

        if (strBoxType->Compare("nymbox")) {
            m.m_lDepth = 0;
        } else if (strBoxType->Compare("inbox")) {
            m.m_lDepth = 1;
        } else if (strBoxType->Compare("outbox")) {
            m.m_lDepth = 2;
        } else {
            m.m_lDepth = 0;
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected boxType to be inbox, outbox, or nymbox, in "
                "getBoxReceiptResponse reply.")
                .Flush();
            return (-1);
        }

        // inReferenceTo contains the getBoxReceipt (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.m_ascInReferenceTo;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "boxReceipt";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload->GetLength())) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected boxReceipt and/or inReferenceTo elements "
                "with text fields in getBoxReceiptResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID: ")(m.m_strAcctID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceiptResponse::reg(
    GET_BOX_RECEIPT_RESPONSE,
    new StrategyGetBoxReceiptResponse());

class StrategyUnregisterAccount final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" AccountID:    ")(
            m.m_strAcctID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request#: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccount::reg(
    UNREGISTER_ACCOUNT,
    new StrategyUnregisterAccount());

class StrategyUnregisterAccountResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, then failure.
        if (!m.m_ascInReferenceTo->GetLength()) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected inReferenceTo element with text fields in "
                "unregisterAccountResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID: ")(m.m_strAcctID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccountResponse::reg(
    UNREGISTER_ACCOUNT_RESPONSE,
    new StrategyUnregisterAccountResponse());

class StrategyNotarizeTransaction final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" AccountID:    ")(
            m.m_strAcctID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request#: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransaction::reg(
    NOTARIZE_TRANSACTION,
    new StrategyNotarizeTransaction());

class StrategyNotarizeTransactionResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }
        if (m.m_bSuccess) {  // Successful message (should contain
                             // responseLedger).
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (!m.m_ascPayload->GetLength() && m.m_bSuccess)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected responseLedger and/or inReferenceTo "
                "elements "
                "with text fields in "
                "notarizeTransactionResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        //      OTString acctContents(m.m_ascPayload);
        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID: ")(m.m_strAcctID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransactionResponse::reg(
    NOTARIZE_TRANSACTION_RESPONSE,
    new StrategyNotarizeTransactionResponse());

class StrategyGetTransactionNumbers final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request#: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbers::reg(
    GET_TRANSACTION_NUMBER,
    new StrategyGetTransactionNumbers());

class StrategyGetTransactionNumbersResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbersResponse::reg(
    GET_TRANSACTION_NUMBER_RESPONSE,
    new StrategyGetTransactionNumbersResponse());

class StrategyGetNymbox final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymbox::reg(GET_NYMBOX, new StrategyGetNymbox());

class StrategyGetNymboxResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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
        if (m.m_bSuccess) {
            pElementExpected = "nymboxLedger";
        } else {
            pElementExpected = "inReferenceTo";
        }

        auto ascTextExpected = Armored::Factory();

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        if (m.m_bSuccess) {
            m.m_ascPayload = ascTextExpected;
        } else {
            m.m_ascInReferenceTo = ascTextExpected;
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymboxResponse::reg(
    GET_NYMBOX_RESPONSE,
    new StrategyGetNymboxResponse());

class StrategyGetAccountData final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("accountID", m.m_strAcctID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID = String::Factory(xml->getAttributeValue("accountID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" AccountID:    ")(m.m_strAcctID.get())(
            " Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountData::reg(
    GET_ACCOUNT_DATA,
    new StrategyGetAccountData());

class StrategyGetAccountDataResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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
            if (!LoadEncodedTextFieldByName(xml, m.m_ascPayload, "account")) {
                LogError()(OT_PRETTY_CLASS())(
                    "Error: Expected account "
                    "element with text field, for ")(m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (!LoadEncodedTextFieldByName(xml, m.m_ascPayload2, "inbox")) {
                LogError()(OT_PRETTY_CLASS())(
                    "Error: Expected inbox"
                    " element with text field, for ")(m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (!LoadEncodedTextFieldByName(xml, m.m_ascPayload3, "outbox")) {
                LogError()(OT_PRETTY_CLASS())(
                    "Error: Expected outbox"
                    " element with text field, for ")(m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        } else {  // Message success=false
            if (!LoadEncodedTextFieldByName(
                    xml, m.m_ascInReferenceTo, "inReferenceTo")) {
                LogError()(OT_PRETTY_CLASS())(
                    "Error: Expected "
                    "inReferenceTo element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID:    ")(m.m_strAcctID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountDataResponse::reg(
    GET_ACCOUNT_DATA_RESPONSE,
    new StrategyGetAccountDataResponse());

class StrategyGetInstrumentDefinition final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("type", std::to_string(m.enum_));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        try {
            m.enum_ = static_cast<std::uint8_t>(
                std::stoi(xml->getAttributeValue("type")));
        } catch (...) {
            m.enum_ = 0;
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Asset Type:    ")(
            m.m_strInstrumentDefinitionID.get())(" Request #: ")(
            m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinition::reg(
    GET_INSTRUMENT_DEFINITION,
    new StrategyGetInstrumentDefinition());

class StrategyGetInstrumentDefinitionResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("found", formatBool(m.m_bBool));
        pTag->add_attribute("type", std::to_string(m.enum_));

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("instrumentDefinition", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto found = String::Factory(xml->getAttributeValue("found"));
        m.m_bBool = found->Compare("true");

        try {
            m.enum_ = static_cast<std::uint8_t>(
                std::stoi(xml->getAttributeValue("type")));
        } catch (...) {
            m.enum_ = 0;
        }

        auto ascTextExpected = Armored::Factory();

        if (false == m.m_bSuccess) {
            const char* pElementExpected = "inReferenceTo";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.m_bBool) {
            const char* pElementExpected = "instrumentDefinition";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.m_bBool) { m.m_ascPayload = ascTextExpected; }

        if (false == m.m_bSuccess) { m.m_ascInReferenceTo = ascTextExpected; }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" Instrument Definition ID:    ")(
            m.m_strInstrumentDefinitionID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinitionResponse::reg(
    GET_INSTRUMENT_DEFINITION_RESPONSE,
    new StrategyGetInstrumentDefinitionResponse());

class StrategyGetMint final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Asset Type:    ")(
            m.m_strInstrumentDefinitionID.get())(" Request #: ")(
            m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMint::reg(GET_MINT, new StrategyGetMint());

class StrategyGetMintResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.m_strInstrumentDefinitionID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("found", formatBool(m.m_bBool));

        if (m.m_ascInReferenceTo->GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        if (m.m_bSuccess && m.m_ascPayload->GetLength()) {
            pTag->add_tag("mint", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strInstrumentDefinitionID =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto found = String::Factory(xml->getAttributeValue("found"));
        m.m_bBool = found->Compare("true");

        auto ascTextExpected = Armored::Factory();

        if (false == m.m_bSuccess) {
            const char* pElementExpected = "inReferenceTo";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bBool) {
            const char* pElementExpected = "mint";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            m.m_ascPayload = ascTextExpected;
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" Instrument Definition ID:    ")(
            m.m_strInstrumentDefinitionID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMintResponse::reg(
    GET_MINT_RESPONSE,
    new StrategyGetMintResponse());

class StrategyProcessInbox final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" AccountID:    ")(
            m.m_strAcctID.get())(" NotaryID: ")(m.m_strNotaryID.get())(
            " Request#: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInbox::reg(
    PROCESS_INBOX,
    new StrategyProcessInbox());

class StrategyProcessInboxResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {  // Success.
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (!m.m_ascPayload->GetLength() && m.m_bSuccess)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected responseLedger and/or inReferenceTo "
                "elements "
                "with text fields in "
                "processInboxResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" AccountID: ")(m.m_strAcctID.get())(
            " NotaryID: ")(m.m_strNotaryID.get())
            .Flush();
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInboxResponse::reg(
    PROCESS_INBOX_RESPONSE,
    new StrategyProcessInboxResponse());

class StrategyProcessNymbox final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request#: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymbox::reg(
    PROCESS_NYMBOX,
    new StrategyProcessNymbox());

class StrategyProcessNymboxResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.m_bSuccess) {  // Success
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.m_ascPayload;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo->GetLength() ||
            (!m.m_ascPayload->GetLength() && m.m_bSuccess)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Expected responseLedger and/or inReferenceTo "
                "elements "
                "with text fields in "
                "processNymboxResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymboxResponse::reg(
    PROCESS_NYMBOX_RESPONSE,
    new StrategyProcessNymboxResponse());

class StrategyTriggerClause final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute(
            "smartContractID", std::to_string(m.m_lTransactionNum));
        pTag->add_attribute("clauseName", m.m_strNymID2->Get());
        pTag->add_attribute("hasParam", formatBool(m.m_ascPayload->Exists()));

        if (m.m_ascPayload->Exists()) {
            pTag->add_tag("parameter", m.m_ascPayload->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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
        if (strTransactionNum->Exists()) {
            m.m_lTransactionNum = strTransactionNum->ToLong();
        }

        if (strHasParam->Compare("true")) {
            const char* pElementExpected = "parameter";
            auto ascTextExpected = Armored::Factory();

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            } else {
                m.m_ascPayload = ascTextExpected;
            }
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Clause TransNum and Name:  ")(
            m.m_lTransactionNum)("  /  ")(m.m_strNymID2.get())(" Request #: ")(
            m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClause::reg(
    TRIGGER_CLAUSE,
    new StrategyTriggerClause());

class StrategyTriggerClauseResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";

        auto ascTextExpected = Armored::Factory();

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        m.m_ascInReferenceTo = ascTextExpected;

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClauseResponse::reg(
    TRIGGER_CLAUSE_RESPONSE,
    new StrategyTriggerClauseResponse());

class StrategyGetMarketList final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())(" Request #: ")(m.m_strRequestNum.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketList::reg(
    GET_MARKET_LIST,
    new StrategyGetMarketList());

class StrategyGetMarketListResponse final : public OTMessageStrategy
{
public:
    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.m_lDepth = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.m_bSuccess) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory();

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.m_bSuccess) {
                m.m_ascPayload->Set(ascTextExpected);
            } else {
                m.m_ascInReferenceTo->Set(ascTextExpected);
            }
        }

        if (m.m_bSuccess) {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
                .Flush();  // m_ascPayload.Get()
        } else {
            LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
                "   ")(m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
                .Flush();  // m_ascInReferenceTo.Get()
        }

        return 1;
    }

    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("depth", std::to_string(m.m_lDepth));

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
    GET_MARKET_LIST_RESPONSE,
    new StrategyGetMarketListResponse());

class StrategyRequestAdmin final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("password", m.m_strAcctID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strAcctID->Set(xml->getAttributeValue("password"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};

RegisterStrategy StrategyRequestAdmin::reg(
    REQUEST_ADMIN,
    new StrategyRequestAdmin());

class StrategyRequestAdminResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("admin", formatBool(m.m_bBool));
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());

        if (m.m_ascInReferenceTo->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        const auto admin = String::Factory(xml->getAttributeValue("admin"));
        m.m_bBool = admin->Compare("true");
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.m_strCommand.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())("  ")(
            m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRequestAdminResponse::reg(
    REQUEST_ADMIN_RESPONSE,
    new StrategyRequestAdminResponse());

class StrategyAddClaim final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
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

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
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

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())(
            " NymID:    ")(m.m_strNymID.get())(" NotaryID: ")(
            m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};

RegisterStrategy StrategyAddClaim::reg(ADD_CLAIM, new StrategyAddClaim());

class StrategyAddClaimResponse final : public OTMessageStrategy
{
public:
    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.m_strCommand->Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum->Get());
        pTag->add_attribute("nymID", m.m_strNymID->Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID->Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash->Get());
        pTag->add_attribute("added", formatBool(m.m_bBool));

        if (false == m.m_bSuccess) {
            if (m.m_ascInReferenceTo->GetLength() > 2) {
                pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo->Get());
            }
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = String::Factory(xml->getNodeName());  // Command
        m.m_strRequestNum =
            String::Factory(xml->getAttributeValue("requestNum"));
        m.m_strNymID = String::Factory(xml->getAttributeValue("nymID"));
        m.m_strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        m.m_strNymboxHash =
            String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto added = String::Factory(xml->getAttributeValue("added"));
        m.m_bBool = added->Compare("true");

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.m_ascInReferenceTo;

        if (false == m.m_bSuccess) {
            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()(OT_PRETTY_CLASS())("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.m_strCommand.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()(OT_PRETTY_CLASS())("Command: ")(m.m_strCommand.get())("  ")(
            m.m_bSuccess ? "SUCCESS" : "FAILURE")(" NymID:    ")(
            m.m_strNymID.get())(" NotaryID: ")(m.m_strNotaryID.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyAddClaimResponse::reg(
    ADD_CLAIM_RESPONSE,
    new StrategyAddClaimResponse());
}  // namespace opentxs
