// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/basket/Basket.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <cstdlib>
#include <memory>

#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/basket/BasketItem.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

// This is a good implementation. Dots all the i's, so to speak.
// client-side.
// The basket ONLY stores closing numbers, so this means "harvest 'em all."
//
// NOTE: The basket might be harvested in different ways, depending on context:
//
// 1. If the command-line client (OR ANY OTHER CLIENT) has a failure BEFORE
// sending the message,
//    (e.g. while constructing the basket exchange request), then it should call
// OTAPI.Msg_HarvestTransactionNumbers
//    and pass in the exchange basket string. That function will check to see if
// the input is an
//    exchange basket, and if so, it will load it up (AS A BASKET) into Basket
// and call the below
//    function to harvest the numbers.
//
// 2. If the high-level API actually SENDS the message, but the message FAILED
// before getting a chance
//    to process the exchangeBasket transaction, then the high-level API will
// pass the failed message
//    to OTAPI.Msg_HarvestTransactionNumbers, which will load it up (AS A
// MESSAGE) and that will then
//    call pMsg->HarvestTransactionNumbers, which then loads up the transaction
// itself in order to call
//    pTransaction->HarvestClosingNumbers. That function, if the transaction is
// indeed an exchangeBasket,
//    will then call the below function Basket::HarvestClosingNumbers.
//
// 3. If the high-level API sends the message, and it SUCCEEDS, but the
// exchangeBasket transaction inside
//    it has FAILED, then OTClient will harvest the transaction numbers when it
// receives the server reply
//    containing the failed transaction, by calling the below function,
// Basket::HarvestClosingNumbers.
//
// 4. If the basket exchange request is constructed successfully, and then the
// message processes at the server
//    successfully, and the transaction inside that message also processed
// successfully, then no harvesting will
//    be performed at all (obviously.)
//

namespace opentxs
{
Basket::Basket(
    const api::Session& api,
    std::int32_t nCount,
    const Amount& lMinimumTransferAmount)
    : Contract(api)
    , sub_count_(nCount)
    , minimum_transfer_(lMinimumTransferAmount)
    , transfer_multiple_(0)
    , request_account_id_()
    , items_()
    , hide_account_id_(false)
    , exchanging_in_(false)
    , closing_transaction_no_(0)
{
}

Basket::Basket(const api::Session& api)
    : Basket(api, 0, 0)
{
}

void Basket::HarvestClosingNumbers(
    otx::context::Server& context,
    const identifier::Notary& theNotaryID,
    bool bSave)
{
    const auto strNotaryID = String::Factory(theNotaryID, api_.Crypto());

    // The SUB-CURRENCIES first...
    const auto nCount = static_cast<std::uint32_t>(Count());

    for (std::uint32_t i = 0; i < nCount; i++) {
        BasketItem* pRequestItem = At(i);

        assert_false(nullptr == pRequestItem);

        const TransactionNumber lClosingTransNo =
            pRequestItem->closing_transaction_no_;

        // This function will only "add it back" if it was really there in the
        // first place. (Verifies it is on issued list first, before adding to
        // available list.)
        context.RecoverAvailableNumber(lClosingTransNo);
    }

    // Then the BASKET currency itself...
    const TransactionNumber lClosingTransNo = GetClosingNum();

    // This function will only "add it back" if it was really there in the first
    // place. (Verifies it is on issued list first, before adding to available
    // list.)
    context.RecoverAvailableNumber(lClosingTransNo);
}

// For generating a user request to EXCHANGE in/out of a basket.
// Assumes that SetTransferMultiple has already been called.
void Basket::AddRequestSubContract(
    const identifier::Generic& SUB_CONTRACT_ID,
    const identifier::Account& SUB_ACCOUNT_ID,
    const std::int64_t& closing_transaction_no)
{
    auto* pItem = new BasketItem;

    assert_false(nullptr == pItem, "Error allocating memory");

    // Minimum transfer amount is not set on a request. The server already knows
    // its value.
    // Also there is no multiple on the item, only on the basket as a whole.
    // ALL items are multiplied by the same multiple. Even the basket amount
    // itself is also.
    items_.push_back(pItem);

    pItem->sub_contract_id_ = SUB_CONTRACT_ID;
    pItem->sub_account_id_ = SUB_ACCOUNT_ID;

    // When the basketReceipts are accepted in all the asset accounts,
    // each one will have a transaction number, closing_transaction_no_,
    // which the user will finally clear from his record by accepting
    // from his inbox.
    pItem->closing_transaction_no_ = closing_transaction_no;
}

// For generating a real basket
void Basket::AddSubContract(
    const identifier::Generic& SUB_CONTRACT_ID,
    std::int64_t lMinimumTransferAmount)
{
    auto* pItem = new BasketItem;

    assert_false(nullptr == pItem, "Error allocating memory");

    pItem->sub_contract_id_ = SUB_CONTRACT_ID;
    pItem->minimum_transfer_amount_ = lMinimumTransferAmount;

    items_.push_back(pItem);
}

// The closing transaction number is the one that gets closed when the
// basketReceipt
// is accepted for the exchange that occured, specific to the basket item at
// nIndex.
// (Each asset account gets its own basketReceipt when an exchange happens.)
//
auto Basket::GetClosingTransactionNoAt(std::uint32_t nIndex) -> std::int64_t
{
    assert_true(nIndex < items_.size(), "index out of bounds");

    BasketItem* pItem = items_.at(nIndex);

    assert_false(nullptr == pItem, "basket item was nullptr at that index");

    return pItem->closing_transaction_no_;
}

auto Basket::At(std::uint32_t nIndex) -> BasketItem*
{
    if (nIndex < items_.size()) { return items_.at(nIndex); }

    return nullptr;
}

auto Basket::Count() const -> std::int32_t
{
    return static_cast<std::int32_t>(items_.size());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto Basket::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("currencyBasket")) {
        auto strSubCount = String::Factory(), strMinTrans = String::Factory();
        strSubCount = String::Factory(xml->getAttributeValue("contractCount"));
        strMinTrans =
            String::Factory(xml->getAttributeValue("minimumTransfer"));

        sub_count_ = atoi(strSubCount->Get());
        minimum_transfer_ = factory::Amount(strMinTrans->Get());

        LogDetail()()("Loading currency basket...").Flush();

        return 1;
    } else if (strNodeName->Compare("requestExchange")) {

        auto strTransferMultiple =
                 String::Factory(xml->getAttributeValue("transferMultiple")),
             strRequestAccountID =
                 String::Factory(xml->getAttributeValue("transferAccountID")),
             strDirection =
                 String::Factory(xml->getAttributeValue("direction")),
             strTemp = String::Factory(
                 xml->getAttributeValue("closingTransactionNo"));

        if (strTransferMultiple->Exists()) {
            transfer_multiple_ = atoi(strTransferMultiple->Get());
        }
        if (strRequestAccountID->Exists()) {
            request_account_id_ = api_.Factory().AccountIDFromBase58(
                strRequestAccountID->Bytes());
        }
        if (strDirection->Exists()) {
            exchanging_in_ = strDirection->Compare("in");
        }
        if (strTemp->Exists()) { SetClosingNum(strTemp->ToLong()); }

        LogVerbose()()("Basket Transfer multiple is ")(
            transfer_multiple_)(". Direction is ")(strDirection.get())(
            ". Closing number is ")(
            closing_transaction_no_)(". Target account is: ")(
            strRequestAccountID.get())
            .Flush();

        return 1;
    } else if (strNodeName->Compare("basketItem")) {
        auto* pItem = new BasketItem;

        assert_false(nullptr == pItem, "Error allocating memory");

        auto strTemp =
            String::Factory(xml->getAttributeValue("minimumTransfer"));
        if (strTemp->Exists()) {
            pItem->minimum_transfer_amount_ = strTemp->ToLong();
        }

        strTemp =
            String::Factory(xml->getAttributeValue("closingTransactionNo"));
        if (strTemp->Exists()) {
            pItem->closing_transaction_no_ = strTemp->ToLong();
        }

        auto strSubAccountID =
                 String::Factory(xml->getAttributeValue("accountID")),
             strContractID = String::Factory(
                 xml->getAttributeValue("instrumentDefinitionID"));
        pItem->sub_account_id_ =
            api_.Factory().AccountIDFromBase58(strSubAccountID->Bytes());
        pItem->sub_contract_id_ =
            api_.Factory().IdentifierFromBase58(strContractID->Bytes());

        items_.push_back(pItem);

        LogVerbose()()("Loaded basket item. ").Flush();

        return 1;
    }

    return 0;
}

// Before transmission or serialization, this is where the basket updates its
// contents
void Basket::UpdateContents(const PasswordPrompt& reason)
{
    GenerateContents(xml_unsigned_, hide_account_id_);
}

void Basket::GenerateContents(StringXML& xmlUnsigned, bool bHideAccountID) const
{
    // I release this because I'm about to repopulate it.
    xmlUnsigned.Release();

    Tag tag("currencyBasket");

    tag.add_attribute("contractCount", std::to_string(sub_count_));
    tag.add_attribute("minimumTransfer", [&] {
        auto buf = UnallocatedCString{};
        minimum_transfer_.Serialize(writer(buf));
        return buf;
    }());

    // Only used in Request Basket (requesting an exchange in/out.)
    // (Versus a basket object used for ISSUING a basket currency, this is
    // EXCHANGING instead.)
    //
    if (IsExchanging()) {
        auto strRequestAcctID =
            String::Factory(request_account_id_, api_.Crypto());

        TagPtr tagRequest(new Tag("requestExchange"));

        tagRequest->add_attribute(
            "transferMultiple", std::to_string(transfer_multiple_));
        tagRequest->add_attribute("transferAccountID", strRequestAcctID->Get());
        tagRequest->add_attribute(
            "closingTransactionNo", std::to_string(closing_transaction_no_));
        tagRequest->add_attribute("direction", exchanging_in_ ? "in" : "out");

        tag.add_tag(tagRequest);
    }

    for (std::int32_t i = 0; i < Count(); i++) {
        BasketItem* pItem = items_[i];

        assert_false(nullptr == pItem, "Error allocating memory");

        auto strAcctID = String::Factory(pItem->sub_account_id_, api_.Crypto()),
             strContractID =
                 String::Factory(pItem->sub_contract_id_, api_.Crypto());

        TagPtr tagItem(new Tag("basketItem"));

        tagItem->add_attribute(
            "minimumTransfer", std::to_string(pItem->minimum_transfer_amount_));
        tagItem->add_attribute(
            "accountID", bHideAccountID ? "" : strAcctID->Get());
        tagItem->add_attribute("instrumentDefinitionID", strContractID->Get());

        if (IsExchanging()) {
            tagItem->add_attribute(
                "closingTransactionNo",
                std::to_string(pItem->closing_transaction_no_));
        }

        tag.add_tag(tagItem);
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xmlUnsigned.Concatenate(String::Factory(str_result));
}

// Most contracts calculate their ID by hashing the Raw File (signatures and
// all).
// The Basket only hashes the unsigned contents, and only with the account IDs
// removed.
// This way, the basket will produce a consistent ID across multiple different
// servers.
void Basket::CalculateContractID(identifier::Generic& newID) const
{
    // Produce a version of the file without account IDs (which are different
    // from server to server.)
    // do this on a copy since we don't want to modify this basket
    auto xmlUnsigned = StringXML::Factory();
    GenerateContents(xmlUnsigned, true);
    newID = api_.Factory().IdentifierFromPreimage(xmlUnsigned->Bytes());
}

void Basket::Release_Basket()
{
    request_account_id_.clear();

    while (!items_.empty()) {
        BasketItem* pItem = items_.front();
        items_.pop_front();
        delete pItem;
    }

    sub_count_ = 0;
    minimum_transfer_ = 0;
    transfer_multiple_ = 0;
    hide_account_id_ = false;
    exchanging_in_ = false;
    closing_transaction_no_ = 0;
}

void Basket::Release()
{
    Release_Basket();

    Contract::Release();
}

Basket::~Basket() { Release_Basket(); }
}  // namespace opentxs
