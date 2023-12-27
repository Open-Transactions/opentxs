// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/Message.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

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
    {otx::MessageType::badID, ERROR_STRING},
    {otx::MessageType::pingNotary, PING_NOTARY},
    {otx::MessageType::pingNotaryResponse, PING_NOTARY_RESPONSE},
    {otx::MessageType::registerNym, REGISTER_NYM},
    {otx::MessageType::registerNymResponse, REGISTER_NYM_RESPONSE},
    {otx::MessageType::unregisterNym, UNREGISTER_NYM},
    {otx::MessageType::unregisterNymResponse, UNREGISTER_NYM_RESPONSE},
    {otx::MessageType::getRequestNumber, GET_REQUEST_NUMBER},
    {otx::MessageType::getRequestNumberResponse, GET_REQUEST_NUMBER_RESPONSE},
    {otx::MessageType::getTransactionNumbers, GET_TRANSACTION_NUMBER},
    {otx::MessageType::getTransactionNumbersResponse,
     GET_TRANSACTION_NUMBER_RESPONSE},
    {otx::MessageType::processNymbox, PROCESS_NYMBOX},
    {otx::MessageType::processNymboxResponse, PROCESS_NYMBOX_RESPONSE},
    {otx::MessageType::checkNym, CHECK_NYM},
    {otx::MessageType::checkNymResponse, CHECK_NYM_RESPONSE},
    {otx::MessageType::sendNymMessage, SEND_NYM_MESSAGE},
    {otx::MessageType::sendNymMessageResponse, SEND_NYM_MESSAGE_RESPONSE},
    {otx::MessageType::sendNymInstrument, SEND_NYM_INSTRUMENT},
    {otx::MessageType::unregisterAccount, UNREGISTER_ACCOUNT},
    {otx::MessageType::unregisterAccountResponse, UNREGISTER_ACCOUNT_RESPONSE},
    {otx::MessageType::registerAccount, REGISTER_ACCOUNT},
    {otx::MessageType::registerAccountResponse, REGISTER_ACCOUNT_RESPONSE},
    {otx::MessageType::registerInstrumentDefinition,
     REGISTER_INSTRUMENT_DEFINITION},
    {otx::MessageType::registerInstrumentDefinitionResponse,
     REGISTER_INSTRUMENT_DEFINITION_RESPONSE},
    {otx::MessageType::issueBasket, ISSUE_BASKET},
    {otx::MessageType::issueBasketResponse, ISSUE_BASKET_RESPONSE},
    {otx::MessageType::notarizeTransaction, NOTARIZE_TRANSACTION},
    {otx::MessageType::notarizeTransactionResponse,
     NOTARIZE_TRANSACTION_RESPONSE},
    {otx::MessageType::getNymbox, GET_NYMBOX},
    {otx::MessageType::getNymboxResponse, GET_NYMBOX_RESPONSE},
    {otx::MessageType::getBoxReceipt, GET_BOX_RECEIPT},
    {otx::MessageType::getBoxReceiptResponse, GET_BOX_RECEIPT_RESPONSE},
    {otx::MessageType::getAccountData, GET_ACCOUNT_DATA},
    {otx::MessageType::getAccountDataResponse, GET_ACCOUNT_DATA_RESPONSE},
    {otx::MessageType::processNymbox, PROCESS_NYMBOX},
    {otx::MessageType::processNymboxResponse, PROCESS_NYMBOX_RESPONSE},
    {otx::MessageType::processInbox, PROCESS_INBOX},
    {otx::MessageType::processInboxResponse, PROCESS_INBOX_RESPONSE},
    {otx::MessageType::queryInstrumentDefinitions, QUERY_INSTRUMENT_DEFINITION},
    {otx::MessageType::queryInstrumentDefinitionsResponse,
     QUERY_INSTRUMENT_DEFINITION_RESPONSE},
    {otx::MessageType::getInstrumentDefinition, GET_INSTRUMENT_DEFINITION},
    {otx::MessageType::getInstrumentDefinitionResponse,
     GET_INSTRUMENT_DEFINITION_RESPONSE},
    {otx::MessageType::getMint, GET_MINT},
    {otx::MessageType::getMintResponse, GET_MINT_RESPONSE},
    {otx::MessageType::getMarketList, GET_MARKET_LIST},
    {otx::MessageType::getMarketListResponse, GET_MARKET_LIST_RESPONSE},
    {otx::MessageType::getMarketOffers, GET_MARKET_OFFERS},
    {otx::MessageType::getMarketOffersResponse, GET_MARKET_OFFERS_RESPONSE},
    {otx::MessageType::getMarketRecentTrades, GET_MARKET_RECENT_TRADES},
    {otx::MessageType::getMarketRecentTradesResponse,
     GET_MARKET_RECENT_TRADES_RESPONSE},
    {otx::MessageType::getNymMarketOffers, GET_NYM_MARKET_OFFERS},
    {otx::MessageType::getNymMarketOffersResponse,
     GET_NYM_MARKET_OFFERS_RESPONSE},
    {otx::MessageType::triggerClause, TRIGGER_CLAUSE},
    {otx::MessageType::triggerClauseResponse, TRIGGER_CLAUSE_RESPONSE},
    {otx::MessageType::usageCredits, USAGE_CREDITS},
    {otx::MessageType::usageCreditsResponse, USAGE_CREDITS_RESPONSE},
    {otx::MessageType::registerContract, REGISTER_CONTRACT},
    {otx::MessageType::registerContractResponse, REGISTER_CONTRACT_RESPONSE},
    {otx::MessageType::requestAdmin, REQUEST_ADMIN},
    {otx::MessageType::requestAdminResponse, REQUEST_ADMIN_RESPONSE},
    {otx::MessageType::addClaim, ADD_CLAIM},
    {otx::MessageType::addClaimResponse, ADD_CLAIM_RESPONSE},
    {otx::MessageType::outmail, OUTMAIL},
};

const UnallocatedMap<otx::MessageType, otx::MessageType>
    Message::reply_message_{
        {otx::MessageType::pingNotary, otx::MessageType::pingNotaryResponse},
        {otx::MessageType::registerNym, otx::MessageType::registerNymResponse},
        {otx::MessageType::unregisterNym,
         otx::MessageType::unregisterNymResponse},
        {otx::MessageType::getRequestNumber,
         otx::MessageType::getRequestNumberResponse},
        {otx::MessageType::getTransactionNumbers,
         otx::MessageType::getTransactionNumbersResponse},
        {otx::MessageType::checkNym, otx::MessageType::checkNymResponse},
        {otx::MessageType::sendNymMessage,
         otx::MessageType::sendNymMessageResponse},
        {otx::MessageType::unregisterAccount,
         otx::MessageType::unregisterAccountResponse},
        {otx::MessageType::registerAccount,
         otx::MessageType::registerAccountResponse},
        {otx::MessageType::registerInstrumentDefinition,
         otx::MessageType::registerInstrumentDefinitionResponse},
        {otx::MessageType::issueBasket, otx::MessageType::issueBasketResponse},
        {otx::MessageType::notarizeTransaction,
         otx::MessageType::notarizeTransactionResponse},
        {otx::MessageType::getNymbox, otx::MessageType::getNymboxResponse},
        {otx::MessageType::getBoxReceipt,
         otx::MessageType::getBoxReceiptResponse},
        {otx::MessageType::getAccountData,
         otx::MessageType::getAccountDataResponse},
        {otx::MessageType::processNymbox,
         otx::MessageType::processNymboxResponse},
        {otx::MessageType::processInbox,
         otx::MessageType::processInboxResponse},
        {otx::MessageType::queryInstrumentDefinitions,
         otx::MessageType::queryInstrumentDefinitionsResponse},
        {otx::MessageType::getInstrumentDefinition,
         otx::MessageType::getInstrumentDefinitionResponse},
        {otx::MessageType::getMint, otx::MessageType::getMintResponse},
        {otx::MessageType::getMarketList,
         otx::MessageType::getMarketListResponse},
        {otx::MessageType::getMarketOffers,
         otx::MessageType::getMarketOffersResponse},
        {otx::MessageType::getMarketRecentTrades,
         otx::MessageType::getMarketRecentTradesResponse},
        {otx::MessageType::getNymMarketOffers,
         otx::MessageType::getNymMarketOffersResponse},
        {otx::MessageType::triggerClause,
         otx::MessageType::triggerClauseResponse},
        {otx::MessageType::usageCredits,
         otx::MessageType::usageCreditsResponse},
        {otx::MessageType::registerContract,
         otx::MessageType::registerContractResponse},
        {otx::MessageType::requestAdmin,
         otx::MessageType::requestAdminResponse},
        {otx::MessageType::addClaim, otx::MessageType::addClaimResponse},
    };

const Message::ReverseTypeMap Message::message_types_ = make_reverse_map();

Message::Message(const api::Session& api)
    : Contract(api)
    , is_signed_(false)
    , command_(String::Factory())
    , notary_id_(String::Factory())
    , nym_id_(String::Factory())
    , nymbox_hash_(String::Factory())
    , inbox_hash_(String::Factory())
    , outbox_hash_(String::Factory())
    , nym_id2_(String::Factory())
    , nym_public_key_(String::Factory())
    , instrument_definition_id_(String::Factory())
    , acct_id_(String::Factory())
    , type_(String::Factory())
    , request_num_(String::Factory())
    , in_reference_to_(Armored::Factory(api_.Crypto()))
    , payload_(Armored::Factory(api_.Crypto()))
    , payload2_(Armored::Factory(api_.Crypto()))
    , payload3_(Armored::Factory(api_.Crypto()))
    , acknowledged_replies_()
    , new_request_num_(0)
    , depth_(0)
    , transaction_num_(0)
    , success_(false)
    , bool_(false)
    , time_(0)
{
    Contract::contract_type_->Set("MESSAGE");
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

auto Message::reply_command(const otx::MessageType& type) -> otx::MessageType
{
    try {

        return reply_message_.at(type);
    } catch (...) {

        return otx::MessageType::badID;
    }
}

auto Message::Command(const otx::MessageType type) -> UnallocatedCString
{
    try {

        return message_names_.at(type);
    } catch (...) {

        return ERROR_STRING;
    }
}

auto Message::Type(const UnallocatedCString& type) -> otx::MessageType
{
    try {

        return message_types_.at(type);
    } catch (...) {

        return otx::MessageType::badID;
    }
}

auto Message::ReplyCommand(const otx::MessageType type) -> UnallocatedCString
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

    const auto MSG_NYM_ID = api_.Factory().NymIDFromBase58(nym_id_->Bytes());
    const auto NOTARY_ID =
        api_.Factory().NotaryIDFromBase58(notary_id_->Bytes());
    const auto ACCOUNT_ID = api_.Factory().AccountIDFromBase58(
        (acct_id_->Exists() ? acct_id_ : nym_id_)->Bytes());  // This
                                                              // may be
    // unnecessary, but just
    // in case.

    const auto strLedger = String::Factory(payload_);
    auto theLedger = api_.Factory().Internal().Session().Ledger(
        MSG_NYM_ID,
        ACCOUNT_ID,
        NOTARY_ID);  // We're going to
                     // load a messsage
                     // ledger from *this.

    if (!strLedger->Exists() || !theLedger->LoadLedgerFromString(strLedger)) {
        LogError()()("ERROR: Failed trying to load message ledger: ")(
            strLedger.get())(".")
            .Flush();
        return false;
    }

    // Let's iterate through the transactions inside, and harvest whatever
    // we can...
    for (const auto& it : theLedger->GetTransactionMap()) {
        auto pTransaction = it.second;
        assert_true(false != bool(pTransaction));

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
    acknowledged_replies_.Release();

    for (const auto& it : numbers) { acknowledged_replies_.Add(it); }
}

// The framework (Contract) will call this function at the appropriate time.
// OTMessage is special because it actually does something here, when most
// contracts are read-only and thus never update their contents.
// Messages, obviously, are different every time, and this function will be
// called just prior to the signing of the message, in Contract::SignContract.
void Message::UpdateContents(const PasswordPrompt& reason)
{
    // I release this because I'm about to repopulate it.
    xml_unsigned_->Release();

    time_ = seconds_since_epoch(Clock::now()).value();

    Tag tag("notaryMessage");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute(
        "dateSigned",
        formatTimestamp(seconds_since_epoch_unsigned(time_).value()));

    if (!updateContentsByType(tag)) {
        TagPtr pTag(new Tag(command_->Get()));
        pTag->add_attribute("requestNum", request_num_->Get());
        pTag->add_attribute("success", formatBool(false));
        pTag->add_attribute("acctID", acct_id_->Get());
        pTag->add_attribute("nymID", nym_id_->Get());
        pTag->add_attribute("notaryID", notary_id_->Get());
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
    if (acknowledged_replies_.Count() > 0) {
        auto strAck = String::Factory();
        if (acknowledged_replies_.Output(strAck) && strAck->Exists()) {
            const auto ascTemp = Armored::Factory(api_.Crypto(), strAck);
            if (ascTemp->Exists()) {
                tag.add_tag("ackReplies", ascTemp->Get());
            }
        }
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
}

auto Message::updateContentsByType(Tag& parent) -> bool
{
    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(command_->Get());
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
    if (!LoadEncodedTextField(api_.Crypto(), xml, strDepth)) {
        LogError()()("Error: ackReplies field "
                     "without value.")
            .Flush();
        return (-1);  // error condition
    }

    acknowledged_replies_.Release();

    if (strDepth->Exists()) { acknowledged_replies_.Add(strDepth); }

    return 1;
}

auto Message::processXmlNodeAcknowledgedReplies(
    [[maybe_unused]] Message& m,
    irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    LogError()()("SKIPPING DEPRECATED FIELD: "
                 "acknowledgedReplies.")
        .Flush();

    while (xml->getNodeType() != irr::io::EXN_ELEMENT_END) { xml->read(); }

    return 1;
}

auto Message::processXmlNodeNotaryMessage(
    [[maybe_unused]] Message& m,
    irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    version_ = String::Factory(xml->getAttributeValue("version"));

    auto strDateSigned = String::Factory(xml->getAttributeValue("dateSigned"));

    if (strDateSigned->Exists()) {
        time_ =
            seconds_since_epoch(parseTimestamp(strDateSigned->Get())).value();
    }

    LogVerbose()()("===> Loading XML for Message into memory structures... ")
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
// We need to update the xml_unsigned_ member with the message members before
// the
// actual signing occurs. (Presumably this is the whole reason why the account
// is being re-signed.)
//
// Normally, in other Contract and derived classes, xml_unsigned_ is read
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
    is_signed_ = Contract::SignContractAuthent(theNym, reason);

    if (false == is_signed_) {
        LogError()()("Failure signing message: ")(xml_unsigned_.get()).Flush();
    }

    return is_signed_;
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
    m.success_ =
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("marketID", m.nym_id2_->Get());
        pTag->add_attribute("depth", std::to_string(m.depth_));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.depth_ = strDepth->ToLong(); }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Market ID: ")(
            m.nym_id2_.get())(" Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("depth", std::to_string(m.depth_));
        pTag->add_attribute("marketID", m.nym_id2_->Get());

        if (m.success_ && (m.payload_->GetLength() > 2) && (m.depth_ > 0)) {
            pTag->add_tag("messagePayload", m.payload_->Get());
        } else if (!m.success_ && (m.in_reference_to_->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.depth_ = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.success_ && (m.depth_ > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.success_) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory(m.api_.Crypto());

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.success_) {
                m.payload_->Set(ascTextExpected);
            } else {
                m.in_reference_to_ = ascTextExpected;
            }
        }

        if (m.success_) {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
                " MarketID: ")(m.nym_id2_.get())
                .Flush();  // payload_.Get()
        } else {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
                " MarketID: ")(m.nym_id2_.get())
                .Flush();  // in_reference_to_.Get()
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("marketID", m.nym_id2_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("marketID"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Market ID: ")(
            m.nym_id2_.get())(" Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("depth", std::to_string(m.depth_));
        pTag->add_attribute("marketID", m.nym_id2_->Get());

        if (m.success_ && (m.payload_->GetLength() > 2) && (m.depth_ > 0)) {
            pTag->add_tag("messagePayload", m.payload_->Get());
        } else if (!m.success_ && (m.in_reference_to_->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("marketID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.depth_ = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.success_ && (m.depth_ > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.success_) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory(m.api_.Crypto());

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.success_) {
                m.payload_->Set(ascTextExpected);
            } else {
                m.in_reference_to_ = ascTextExpected;
            }
        }

        if (m.success_) {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
                " MarketID: ")(m.nym_id2_.get())
                .Flush();  // payload_.Get()
        } else {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
                " MarketID: ")(m.nym_id2_.get())
                .Flush();  // in_reference_to_.Get()
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request #: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("depth", std::to_string(m.depth_));

        if (m.success_ && (m.payload_->GetLength() > 2) && (m.depth_ > 0)) {
            pTag->add_tag("messagePayload", m.payload_->Get());
        } else if (!m.success_ && (m.in_reference_to_->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.depth_ = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.success_ && (m.depth_ > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.success_) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory(m.api_.Crypto());

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.success_) {
                m.payload_->Set(ascTextExpected);
            } else {
                m.in_reference_to_ = ascTextExpected;
            }
        }

        if (m.success_) {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
                .Flush();  // payload_.Get()
        } else {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
                .Flush();  // in_reference_to_.Get()
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        TagPtr pAuthentKeyTag(
            new Tag("publicAuthentKey", m.nym_public_key_->Get()));
        TagPtr pEncryptKeyTag(
            new Tag("publicEncryptionKey", m.nym_id2_->Get()));

        pAuthentKeyTag->add_attribute("type", "notused");
        pEncryptKeyTag->add_attribute("type", "notused");

        pTag->add_tag(pAuthentKeyTag);
        pTag->add_tag(pEncryptKeyTag);

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        // -------------------------------------------------
        const char* pElementExpected = "publicAuthentKey";
        auto ascTextExpected = Armored::Factory(m.api_.Crypto());

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
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        m.nym_public_key_->Set(ascTextExpected);

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
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        m.nym_id2_->Set(ascTextExpected);

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
            " Public signing key: ")(m.nym_public_key_.get())(
            " Public encryption key: ")(m.nym_id2_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        LogDetail()()("Command: ")(m.command_.get())(" Success: ")(
            m.success_ ? "true" : "false")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("contract", m.payload_->Get());
        pTag->add_attribute("type", std::to_string(m.enum_));
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.payload_->Set(xml->getAttributeValue("contract"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        try {
            m.enum_ = static_cast<std::uint8_t>(
                std::stoi(xml->getAttributeValue("type")));
        } catch (...) {
            m.enum_ = 0;
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        if (m.in_reference_to_->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.in_reference_to_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("  ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("publicnym", m.payload_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.payload_->Set(xml->getAttributeValue("publicnym"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        if (m.success_ && (m.payload_->GetLength() > 2)) {
            pTag->add_tag("nymfile", m.payload_->Get());
        }

        if (m.in_reference_to_->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        if (m.success_) {
            const char* pElementExpected = "nymfile";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.in_reference_to_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("  ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        if (m.in_reference_to_->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.in_reference_to_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("  ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NymID2:    ")(m.nym_id2_.get())(" NotaryID: ")(
            m.notary_id_.get())(" Request #: ")(m.request_num_.get())
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
        const bool bCredentials = (m.payload_->Exists());
        assert_true(!m.bool_ || bCredentials);

        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("found", formatBool(m.bool_));

        if (m.bool_ && bCredentials) {
            pTag->add_tag("publicnym", m.payload_->Get());
        }

        if (false == m.success_) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto found = String::Factory(xml->getAttributeValue("found"));
        m.bool_ = found->Compare("true");

        auto ascTextExpected = Armored::Factory(m.api_.Crypto());
        const char* pElementExpected = nullptr;

        if (!m.success_) {
            pElementExpected = "inReferenceTo";
            m.in_reference_to_ = ascTextExpected;
            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        } else if (m.bool_) {  // Success.
            pElementExpected = "publicnym";
            ascTextExpected->Release();

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
            m.payload_ = ascTextExpected;
        }

        if (m.bool_) {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NymID2:    ")(m.nym_id2_.get())(
                " NotaryID: ")(m.notary_id_.get())(" Nym2 Public Key: ")(
                m.nym_public_key_.get())
                .Flush();
        } else {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NymID2:    ")(m.nym_id2_.get())(
                " NotaryID: ")(m.notary_id_.get())
                .Flush();
        }
        // m.in_reference_to_.Get()

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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("adjustment", std::to_string(m.depth_));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        auto strAdjustment =
            String::Factory(xml->getAttributeValue("adjustment"));

        if (strAdjustment->GetLength() > 0) {
            m.depth_ = strAdjustment->ToLong();
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NymID2:    ")(m.nym_id2_.get())(" NotaryID: ")(
            m.notary_id_.get())(" Request #: ")(m.request_num_.get())(
            " Adjustment: ")(m.depth_)
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("totalCredits", std::to_string(m.depth_));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        auto strTotalCredits =
            String::Factory(xml->getAttributeValue("totalCredits"));

        if (strTotalCredits->GetLength() > 0) {
            m.depth_ = strTotalCredits->ToLong();
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NymID2:    ")(m.nym_id2_.get())(" NotaryID: ")(
            m.notary_id_.get())(" Total Credits: ")(m.depth_)
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        if (m.payload_->GetLength() > 2) {
            pTag->add_tag("messagePayload", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.payload_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NymID2:    ")(m.nym_id2_.get())(" NotaryID: ")(
            m.notary_id_.get())(" Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        if (m.payload_->GetLength() > 2) {
            pTag->add_tag("messagePayload", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.payload_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NymID2:    ")(m.nym_id2_.get())(" NotaryID: ")(
            m.notary_id_.get())(" Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NymID2:    ")(m.nym_id2_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("nymID2", m.nym_id2_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        if (m.payload_->GetLength() > 2) {
            pTag->add_tag("messagePayload", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("nymID2"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "messagePayload";
        Armored& ascTextExpected = m.payload_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NymID2:    ")(m.nym_id2_.get())(" NotaryID: ")(
            m.notary_id_.get())(" Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "newRequestNum", std::to_string(m.new_request_num_));
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        const auto strNewRequestNum =
            String::Factory(xml->getAttributeValue("newRequestNum"));
        m.new_request_num_ =
            strNewRequestNum->Exists() ? strNewRequestNum->ToLong() : 0;

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())(" Request Number:    ")(
            m.request_num_.get())(" New Number: ")(m.new_request_num_)
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());

        if (m.payload_->GetLength()) {
            pTag->add_tag("instrumentDefinition", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "instrumentDefinition";
        Armored& ascTextExpected = m.payload_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())(" Asset Type: ")(
            m.instrument_definition_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());
        // the new issuer account ID
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("issuerAccount", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.success_) {
            const char* pElementExpected = "issuerAccount";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.in_reference_to_->GetLength() ||
            (m.success_ && !m.payload_->GetLength())) {
            LogError()()("Error: "
                         "Expected issuerAccount and/or inReferenceTo elements "
                         "with text fields in "
                         "registerInstrumentDefinitionResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        auto acctContents = String::Factory(m.payload_);
        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID: ")(m.acct_id_.get())(" Instrument Definition ID: ")(
            m.instrument_definition_id_.get())(" NotaryID: ")(
            m.notary_id_.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.in_reference_to_.Get(),
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        if (m.payload_->GetLength()) {
            pTag->add_tag("stringMap", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        const char* pElementExpected = "stringMap";
        Armored& ascTextExpected = m.payload_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("stringMap", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.success_) {
            const char* pElementExpected = "stringMap";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.in_reference_to_->GetLength() ||
            (m.success_ && !m.payload_->GetLength())) {
            LogError()()(
                "Error: Expected stringMap and/or inReferenceTo elements "
                "with text fields in "
                "queryInstrumentDefinitionsResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        if (m.payload_->GetLength()) {
            pTag->add_tag("currencyBasket", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "currencyBasket";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the Payload isn't there, then failure.
        if (!m.payload_->GetLength()) {
            LogError()()(
                "Error: Expected currencyBasket element with text fields in "
                "issueBasket message.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.in_reference_to_->GetLength()) {
            LogError()()(
                "Error: Expected inReferenceTo element with text fields in "
                "issueBasketResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID: ")(m.acct_id_.get())(" InstrumentDefinitionID: ")(
            m.instrument_definition_id_.get())(" NotaryID: ")(
            m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())(" Asset Type: ")(
            m.instrument_definition_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.in_reference_to_->Exists()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_ && m.payload_->Exists()) {
            pTag->add_tag("newAccount", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                //                return (-1); // error condition
            }
        }

        if (m.success_) {
            const char* pElementExpected = "newAccount";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        //
        if (m.success_ && !m.payload_->GetLength()) {
            LogError()()(
                "Error: Expected newAccount element with text field, in "
                "registerAccountResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID: ")(m.acct_id_.get())(" NotaryID: ")(m.notary_id_.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.in_reference_to_.Get(),
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        // If retrieving box receipt for Nymbox, NymID
        // will appear in this variable.
        pTag->add_attribute("accountID", m.acct_id_->Get());
        pTag->add_attribute(
            "boxType",  // outbox is 2.
            (m.depth_ == 0) ? "nymbox"
                            : ((m.depth_ == 1) ? "inbox" : "outbox"));
        pTag->add_attribute(
            "transactionNum", std::to_string(m.transaction_num_));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        auto strTransactionNum =
            String::Factory(xml->getAttributeValue("transactionNum"));
        m.transaction_num_ =
            strTransactionNum->Exists() ? strTransactionNum->ToLong() : 0;

        const auto strBoxType =
            String::Factory(xml->getAttributeValue("boxType"));

        if (strBoxType->Compare("nymbox")) {
            m.depth_ = 0;
        } else if (strBoxType->Compare("inbox")) {
            m.depth_ = 1;
        } else if (strBoxType->Compare("outbox")) {
            m.depth_ = 2;
        } else {
            m.depth_ = 0;
            LogError()()(
                "Error: Expected boxType to be inbox, outbox, or nymbox, in "
                "getBoxReceipt.")
                .Flush();
            return (-1);
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" AccountID:    ")(m.acct_id_.get())(
            " NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())(" Transaction#: ")(m.transaction_num_)(
            " boxType: ")(((m.depth_ == 0)   ? "nymbox"
                           : (m.depth_ == 1) ? "inbox"
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());
        pTag->add_attribute(
            "boxType",  // outbox is 2.
            (m.depth_ == 0) ? "nymbox"
                            : ((m.depth_ == 1) ? "inbox" : "outbox"));
        pTag->add_attribute(
            "transactionNum", std::to_string(m.transaction_num_));

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("boxReceipt", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));

        auto strTransactionNum =
            String::Factory(xml->getAttributeValue("transactionNum"));
        m.transaction_num_ =
            strTransactionNum->Exists() ? strTransactionNum->ToLong() : 0;

        const auto strBoxType =
            String::Factory(xml->getAttributeValue("boxType"));

        if (strBoxType->Compare("nymbox")) {
            m.depth_ = 0;
        } else if (strBoxType->Compare("inbox")) {
            m.depth_ = 1;
        } else if (strBoxType->Compare("outbox")) {
            m.depth_ = 2;
        } else {
            m.depth_ = 0;
            LogError()()(
                "Error: Expected boxType to be inbox, outbox, or nymbox, in "
                "getBoxReceiptResponse reply.")
                .Flush();
            return (-1);
        }

        // inReferenceTo contains the getBoxReceipt (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.success_) {
            const char* pElementExpected = "boxReceipt";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.in_reference_to_->GetLength() ||
            (m.success_ && !m.payload_->GetLength())) {
            LogError()()(
                "Error: Expected boxReceipt and/or inReferenceTo elements "
                "with text fields in getBoxReceiptResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID: ")(m.acct_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" AccountID:    ")(m.acct_id_.get())(
            " NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));

        // inReferenceTo contains the unregisterAccount (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, then failure.
        if (!m.in_reference_to_->GetLength()) {
            LogError()()(
                "Error: Expected inReferenceTo element with text fields in "
                "unregisterAccountResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID: ")(m.acct_id_.get())(" NotaryID: ")(m.notary_id_.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.in_reference_to_.Get(),
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.payload_->GetLength()) {
            pTag->add_tag("accountLedger", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "accountLedger";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" AccountID:    ")(m.acct_id_.get())(
            " NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }
        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("responseLedger", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.
        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }
        if (m.success_) {  // Successful message (should contain
                           // responseLedger).
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.in_reference_to_->GetLength() ||
            (!m.payload_->GetLength() && m.success_)) {
            LogError()()("Error: Expected responseLedger and/or inReferenceTo "
                         "elements "
                         "with text fields in "
                         "notarizeTransactionResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        //      OTString acctContents(m.payload_);
        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID: ")(m.acct_id_.get())(" NotaryID: ")(m.notary_id_.get())
            .Flush();
        //    "****New Account****:\n%s\n",
        //    m.in_reference_to_.Get(),
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request #: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("nymboxLedger", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected;
        if (m.success_) {
            pElementExpected = "nymboxLedger";
        } else {
            pElementExpected = "inReferenceTo";
        }

        auto ascTextExpected = Armored::Factory(m.api_.Crypto());

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        if (m.success_) {
            m.payload_ = ascTextExpected;
        } else {
            m.in_reference_to_ = ascTextExpected;
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
            " AccountID:    ")(m.acct_id_.get())(" Request #: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());
        pTag->add_attribute("inboxHash", m.inbox_hash_->Get());
        pTag->add_attribute("outboxHash", m.outbox_hash_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_) {
            if (m.payload_->GetLength()) {
                pTag->add_tag("account", m.payload_->Get());
            }
            if (m.payload2_->GetLength()) {
                pTag->add_tag("inbox", m.payload2_->Get());
            }
            if (m.payload3_->GetLength()) {
                pTag->add_tag("outbox", m.payload3_->Get());
            }
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));
        m.inbox_hash_ = String::Factory(xml->getAttributeValue("inboxHash"));
        m.outbox_hash_ = String::Factory(xml->getAttributeValue("outboxHash"));

        if (m.success_) {
            if (!LoadEncodedTextFieldByName(xml, m.payload_, "account")) {
                LogError()()(
                    "Error: Expected account "
                    "element with text field, for ")(m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (!LoadEncodedTextFieldByName(xml, m.payload2_, "inbox")) {
                LogError()()(
                    "Error: Expected inbox"
                    " element with text field, for ")(m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (!LoadEncodedTextFieldByName(xml, m.payload3_, "outbox")) {
                LogError()()(
                    "Error: Expected outbox"
                    " element with text field, for ")(m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        } else {  // Message success=false
            if (!LoadEncodedTextFieldByName(
                    xml, m.in_reference_to_, "inReferenceTo")) {
                LogError()()("Error: Expected "
                             "inReferenceTo element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID:    ")(m.acct_id_.get())(" NotaryID: ")(
            m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("type", std::to_string(m.enum_));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        try {
            m.enum_ = static_cast<std::uint8_t>(
                std::stoi(xml->getAttributeValue("type")));
        } catch (...) {
            m.enum_ = 0;
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
            " Asset Type:    ")(m.instrument_definition_id_.get())(
            " Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("found", formatBool(m.bool_));
        pTag->add_attribute("type", std::to_string(m.enum_));

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("instrumentDefinition", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto found = String::Factory(xml->getAttributeValue("found"));
        m.bool_ = found->Compare("true");

        try {
            m.enum_ = static_cast<std::uint8_t>(
                std::stoi(xml->getAttributeValue("type")));
        } catch (...) {
            m.enum_ = 0;
        }

        auto ascTextExpected = Armored::Factory(m.api_.Crypto());

        if (false == m.success_) {
            const char* pElementExpected = "inReferenceTo";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.bool_) {
            const char* pElementExpected = "instrumentDefinition";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.bool_) { m.payload_ = ascTextExpected; }

        if (false == m.success_) { m.in_reference_to_ = ascTextExpected; }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " Instrument Definition ID:    ")(
            m.instrument_definition_id_.get())(" NotaryID: ")(
            m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
            " Asset Type:    ")(m.instrument_definition_id_.get())(
            " Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute(
            "instrumentDefinitionID", m.instrument_definition_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("found", formatBool(m.bool_));

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("mint", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.instrument_definition_id_ =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto found = String::Factory(xml->getAttributeValue("found"));
        m.bool_ = found->Compare("true");

        auto ascTextExpected = Armored::Factory(m.api_.Crypto());

        if (false == m.success_) {
            const char* pElementExpected = "inReferenceTo";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            m.in_reference_to_ = ascTextExpected;
        }

        if (m.bool_) {
            const char* pElementExpected = "mint";

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            m.payload_ = ascTextExpected;
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " Instrument Definition ID:    ")(
            m.instrument_definition_id_.get())(" NotaryID: ")(
            m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.payload_->GetLength()) {
            pTag->add_tag("processLedger", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "processLedger";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" AccountID:    ")(m.acct_id_.get())(
            " NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("accountID", m.acct_id_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }
        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("responseLedger", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.acct_id_ = String::Factory(xml->getAttributeValue("accountID"));

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.
        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.success_) {  // Success.
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.in_reference_to_->GetLength() ||
            (!m.payload_->GetLength() && m.success_)) {
            LogError()()("Error: Expected responseLedger and/or inReferenceTo "
                         "elements "
                         "with text fields in "
                         "processInboxResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " AccountID: ")(m.acct_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        if (m.payload_->GetLength()) {
            pTag->add_tag("processLedger", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        {
            const char* pElementExpected = "processLedger";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request#: ")(
            m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }
        if (m.success_ && m.payload_->GetLength()) {
            pTag->add_tag("responseLedger", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.
        {
            const char* pElementExpected = "inReferenceTo";
            Armored& ascTextExpected = m.in_reference_to_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        if (m.success_) {  // Success
            const char* pElementExpected = "responseLedger";
            Armored& ascTextExpected = m.payload_;

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.in_reference_to_->GetLength() ||
            (!m.payload_->GetLength() && m.success_)) {
            LogError()()("Error: Expected responseLedger and/or inReferenceTo "
                         "elements "
                         "with text fields in "
                         "processNymboxResponse reply.")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute(
            "smartContractID", std::to_string(m.transaction_num_));
        pTag->add_attribute("clauseName", m.nym_id2_->Get());
        pTag->add_attribute("hasParam", formatBool(m.payload_->Exists()));

        if (m.payload_->Exists()) {
            pTag->add_tag("parameter", m.payload_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nym_id2_ = String::Factory(xml->getAttributeValue("clauseName"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        const auto strHasParam =
            String::Factory(xml->getAttributeValue("hasParam"));

        auto strTransactionNum =
            String::Factory(xml->getAttributeValue("smartContractID"));
        if (strTransactionNum->Exists()) {
            m.transaction_num_ = strTransactionNum->ToLong();
        }

        if (strHasParam->Compare("true")) {
            const char* pElementExpected = "parameter";
            auto ascTextExpected = Armored::Factory(m.api_.Crypto());

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            } else {
                m.payload_ = ascTextExpected;
            }
        }

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(
            " Clause TransNum and Name:  ")(m.transaction_num_)("  /  ")(
            m.nym_id2_.get())(" Request #: ")(m.request_num_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        if (m.in_reference_to_->GetLength()) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        const char* pElementExpected = "inReferenceTo";

        auto ascTextExpected = Armored::Factory(m.api_.Crypto());

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        m.in_reference_to_ = ascTextExpected;

        LogDetail()()("Command: ")(m.command_.get())("   ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())(" Request #: ")(
            m.request_num_.get())
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

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));

        auto strDepth = String::Factory(xml->getAttributeValue("depth"));

        if (strDepth->GetLength() > 0) { m.depth_ = strDepth->ToLong(); }

        const char* pElementExpected = nullptr;
        if (m.success_ && (m.depth_ > 0)) {
            pElementExpected = "messagePayload";
        } else if (!m.success_) {
            pElementExpected = "inReferenceTo";
        }

        if (nullptr != pElementExpected) {
            auto ascTextExpected = Armored::Factory(m.api_.Crypto());

            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }

            if (m.success_) {
                m.payload_->Set(ascTextExpected);
            } else {
                m.in_reference_to_->Set(ascTextExpected);
            }
        }

        if (m.success_) {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
                .Flush();  // payload_.Get()
        } else {
            LogDetail()()("Command: ")(m.command_.get())("   ")(
                m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(
                m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
                .Flush();  // in_reference_to_.Get()
        }

        return 1;
    }

    void writeXml(Message& m, Tag& parent) final
    {
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("depth", std::to_string(m.depth_));

        if (m.success_ && (m.payload_->GetLength() > 2) && (m.depth_ > 0)) {
            pTag->add_tag("messagePayload", m.payload_->Get());
        } else if (!m.success_ && (m.in_reference_to_->GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("password", m.acct_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.acct_id_->Set(xml->getAttributeValue("password"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("admin", formatBool(m.bool_));
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());

        if (m.in_reference_to_->GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        const auto admin = String::Factory(xml->getAttributeValue("admin"));
        m.bool_ = admin->Compare("true");
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.in_reference_to_;

        if (!LoadEncodedTextFieldByName(
                xml, ascTextExpected, pElementExpected)) {
            LogError()()("Error: Expected ")(
                pElementExpected)(" element with text field, for ")(
                m.command_.get())(".")
                .Flush();
            return (-1);  // error condition
        }

        LogDetail()()("Command: ")(m.command_.get())("  ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("section", m.nym_id2_->Get());
        pTag->add_attribute("type", m.instrument_definition_id_->Get());
        pTag->add_attribute("value", m.acct_id_->Get());
        pTag->add_attribute("primary", formatBool(m.bool_));

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nym_id2_->Set(xml->getAttributeValue("section"));
        m.instrument_definition_id_->Set(xml->getAttributeValue("type"));
        m.acct_id_->Set(xml->getAttributeValue("value"));
        const auto primary = String::Factory(xml->getAttributeValue("primary"));
        m.bool_ = primary->Compare("true");

        LogDetail()()("Command: ")(m.command_.get())(" NymID:    ")(
            m.nym_id_.get())(" NotaryID: ")(m.notary_id_.get())
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
        TagPtr pTag(new Tag(m.command_->Get()));

        pTag->add_attribute("success", formatBool(m.success_));
        pTag->add_attribute("requestNum", m.request_num_->Get());
        pTag->add_attribute("nymID", m.nym_id_->Get());
        pTag->add_attribute("notaryID", m.notary_id_->Get());
        pTag->add_attribute("nymboxHash", m.nymbox_hash_->Get());
        pTag->add_attribute("added", formatBool(m.bool_));

        if (false == m.success_) {
            if (m.in_reference_to_->GetLength() > 2) {
                pTag->add_tag("inReferenceTo", m.in_reference_to_->Get());
            }
        }

        parent.add_tag(pTag);
    }

    auto processXml(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t final
    {
        processXmlSuccess(m, xml);

        m.command_ = String::Factory(xml->getNodeName());  // Command
        m.request_num_ = String::Factory(xml->getAttributeValue("requestNum"));
        m.nym_id_ = String::Factory(xml->getAttributeValue("nymID"));
        m.notary_id_ = String::Factory(xml->getAttributeValue("notaryID"));
        m.nymbox_hash_ = String::Factory(xml->getAttributeValue("nymboxHash"));
        const auto added = String::Factory(xml->getAttributeValue("added"));
        m.bool_ = added->Compare("true");

        const char* pElementExpected = "inReferenceTo";
        Armored& ascTextExpected = m.in_reference_to_;

        if (false == m.success_) {
            if (!LoadEncodedTextFieldByName(
                    xml, ascTextExpected, pElementExpected)) {
                LogError()()("Error: Expected ")(
                    pElementExpected)(" element with text field, for ")(
                    m.command_.get())(".")
                    .Flush();
                return (-1);  // error condition
            }
        }

        LogDetail()()("Command: ")(m.command_.get())("  ")(
            m.success_ ? "SUCCESS" : "FAILURE")(" NymID:    ")(m.nym_id_.get())(
            " NotaryID: ")(m.notary_id_.get())
            .Flush();

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyAddClaimResponse::reg(
    ADD_CLAIM_RESPONSE,
    new StrategyAddClaimResponse());
}  // namespace opentxs
