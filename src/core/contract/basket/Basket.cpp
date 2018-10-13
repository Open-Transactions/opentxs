// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/basket/Basket.hpp"

#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/contract/basket/BasketItem.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::Basket"

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
    const api::Core& core,
    std::int32_t nCount,
    std::int64_t lMinimumTransferAmount)
    : Contract(core)
    , m_nSubCount(nCount)
    , m_lMinimumTransfer(lMinimumTransferAmount)
    , m_nTransferMultiple(0)
    , m_RequestAccountID(Identifier::Factory())
    , m_dequeItems()
    , m_bHideAccountID(false)
    , m_bExchangingIn(false)
    , m_lClosingTransactionNo(0)
{
}

Basket::Basket(const api::Core& core)
    : Basket(core, 0, 0)
{
}

void Basket::HarvestClosingNumbers(
    ServerContext& context,
    const Identifier& theNotaryID,
    bool bSave)
{
    const auto strNotaryID = String::Factory(theNotaryID);

    // The SUB-CURRENCIES first...
    const std::uint32_t nCount = static_cast<uint32_t>(Count());

    for (std::uint32_t i = 0; i < nCount; i++) {
        BasketItem* pRequestItem = At(i);

        OT_ASSERT(nullptr != pRequestItem);

        const TransactionNumber lClosingTransNo =
            pRequestItem->lClosingTransactionNo;

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
    const Identifier& SUB_CONTRACT_ID,
    const Identifier& SUB_ACCOUNT_ID,
    const std::int64_t& lClosingTransactionNo)
{
    BasketItem* pItem = new BasketItem;

    OT_ASSERT_MSG(
        nullptr != pItem,
        "Error allocating memory in Basket::AddRequestSubContract\n");

    // Minimum transfer amount is not set on a request. The server already knows
    // its value.
    // Also there is no multiple on the item, only on the basket as a whole.
    // ALL items are multiplied by the same multiple. Even the basket amount
    // itself is also.
    m_dequeItems.push_back(pItem);

    pItem->SUB_CONTRACT_ID = SUB_CONTRACT_ID;
    pItem->SUB_ACCOUNT_ID = SUB_ACCOUNT_ID;

    // When the basketReceipts are accepted in all the asset accounts,
    // each one will have a transaction number, lClosingTransactionNo,
    // which the user will finally clear from his record by accepting
    // from his inbox.
    pItem->lClosingTransactionNo = lClosingTransactionNo;
}

// For generating a real basket
void Basket::AddSubContract(
    const Identifier& SUB_CONTRACT_ID,
    std::int64_t lMinimumTransferAmount)
{
    BasketItem* pItem = new BasketItem;

    OT_ASSERT_MSG(
        nullptr != pItem,
        "Error allocating memory in Basket::AddSubContract\n");

    pItem->SUB_CONTRACT_ID = SUB_CONTRACT_ID;
    pItem->lMinimumTransferAmount = lMinimumTransferAmount;

    m_dequeItems.push_back(pItem);
}

// The closing transaction number is the one that gets closed when the
// basketReceipt
// is accepted for the exchange that occured, specific to the basket item at
// nIndex.
// (Each asset account gets its own basketReceipt when an exchange happens.)
//
std::int64_t Basket::GetClosingTransactionNoAt(std::uint32_t nIndex)
{
    OT_ASSERT_MSG(
        (nIndex < m_dequeItems.size()),
        "Basket::GetClosingTransactionNoAt: index out of bounds.");

    BasketItem* pItem = m_dequeItems.at(nIndex);

    OT_ASSERT_MSG(
        nullptr != pItem,
        "Basket::GetClosingTransactionNoAt: basket "
        "item was nullptr at that index.");

    return pItem->lClosingTransactionNo;
}

BasketItem* Basket::At(std::uint32_t nIndex)
{
    if (nIndex < m_dequeItems.size()) return m_dequeItems.at(nIndex);

    return nullptr;
}

std::int32_t Basket::Count() const
{
    return static_cast<std::int32_t>(m_dequeItems.size());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Basket::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("currencyBasket")) {
        auto strSubCount = String::Factory(), strMinTrans = String::Factory();
        strSubCount = String::Factory(xml->getAttributeValue("contractCount"));
        strMinTrans =
            String::Factory(xml->getAttributeValue("minimumTransfer"));

        m_nSubCount = atoi(strSubCount->Get());
        m_lMinimumTransfer = strMinTrans->ToLong();

        LogDetail(OT_METHOD)(__FUNCTION__)("Loading currency basket... ")
            .Flush();

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

        if (strTransferMultiple->Exists())
            m_nTransferMultiple = atoi(strTransferMultiple->Get());
        if (strRequestAccountID->Exists())
            m_RequestAccountID->SetString(strRequestAccountID);
        if (strDirection->Exists())
            m_bExchangingIn = strDirection->Compare("in");
        if (strTemp->Exists()) SetClosingNum(strTemp->ToLong());

        LogVerbose(OT_METHOD)(__FUNCTION__)("Basket Transfer multiple is ")(
            m_nTransferMultiple)(". Direction is ")(strDirection)(
            ". Closing number is ")(m_lClosingTransactionNo)(
            ". Target account is: ")(strRequestAccountID)
            .Flush();

        return 1;
    } else if (strNodeName->Compare("basketItem")) {
        BasketItem* pItem = new BasketItem;

        OT_ASSERT_MSG(
            nullptr != pItem,
            "Error allocating memory in Basket::ProcessXMLNode\n");

        auto strTemp =
            String::Factory(xml->getAttributeValue("minimumTransfer"));
        if (strTemp->Exists())
            pItem->lMinimumTransferAmount = strTemp->ToLong();

        strTemp =
            String::Factory(xml->getAttributeValue("closingTransactionNo"));
        if (strTemp->Exists()) pItem->lClosingTransactionNo = strTemp->ToLong();

        auto strSubAccountID =
                 String::Factory(xml->getAttributeValue("accountID")),
             strContractID = String::Factory(
                 xml->getAttributeValue("instrumentDefinitionID"));
        pItem->SUB_ACCOUNT_ID->SetString(strSubAccountID);
        pItem->SUB_CONTRACT_ID->SetString(strContractID);

        m_dequeItems.push_back(pItem);

        LogVerbose(OT_METHOD)(__FUNCTION__)("Loaded basket item. ").Flush();

        return 1;
    }

    return 0;
}

// Before transmission or serialization, this is where the basket updates its
// contents
void Basket::UpdateContents()
{
    GenerateContents(m_xmlUnsigned, m_bHideAccountID);
}

void Basket::GenerateContents(StringXML& xmlUnsigned, bool bHideAccountID) const
{
    // I release this because I'm about to repopulate it.
    xmlUnsigned.Release();

    Tag tag("currencyBasket");

    tag.add_attribute("contractCount", formatInt(m_nSubCount));
    tag.add_attribute("minimumTransfer", formatLong(m_lMinimumTransfer));

    // Only used in Request Basket (requesting an exchange in/out.)
    // (Versus a basket object used for ISSUING a basket currency, this is
    // EXCHANGING instead.)
    //
    if (IsExchanging()) {
        auto strRequestAcctID = String::Factory(m_RequestAccountID);

        TagPtr tagRequest(new Tag("requestExchange"));

        tagRequest->add_attribute(
            "transferMultiple", formatInt(m_nTransferMultiple));
        tagRequest->add_attribute("transferAccountID", strRequestAcctID->Get());
        tagRequest->add_attribute(
            "closingTransactionNo", formatLong(m_lClosingTransactionNo));
        tagRequest->add_attribute("direction", m_bExchangingIn ? "in" : "out");

        tag.add_tag(tagRequest);
    }

    for (std::int32_t i = 0; i < Count(); i++) {
        BasketItem* pItem = m_dequeItems[i];

        OT_ASSERT_MSG(
            nullptr != pItem,
            "Error allocating memory in Basket::UpdateContents\n");

        auto strAcctID = String::Factory(pItem->SUB_ACCOUNT_ID),
             strContractID = String::Factory(pItem->SUB_CONTRACT_ID);

        TagPtr tagItem(new Tag("basketItem"));

        tagItem->add_attribute(
            "minimumTransfer", formatLong(pItem->lMinimumTransferAmount));
        tagItem->add_attribute(
            "accountID", bHideAccountID ? "" : strAcctID->Get());
        tagItem->add_attribute("instrumentDefinitionID", strContractID->Get());

        if (IsExchanging()) {
            tagItem->add_attribute(
                "closingTransactionNo",
                formatLong(pItem->lClosingTransactionNo));
        }

        tag.add_tag(tagItem);
    }

    std::string str_result;
    tag.output(str_result);

    xmlUnsigned.Concatenate("%s", str_result.c_str());
}

// Most contracts calculate their ID by hashing the Raw File (signatures and
// all).
// The Basket only hashes the unsigned contents, and only with the account IDs
// removed.
// This way, the basket will produce a consistent ID across multiple different
// servers.
void Basket::CalculateContractID(Identifier& newID) const
{
    // Produce a version of the file without account IDs (which are different
    // from server to server.)
    // do this on a copy since we don't want to modify this basket
    auto xmlUnsigned = StringXML::Factory();
    GenerateContents(xmlUnsigned, true);
    newID.CalculateDigest(xmlUnsigned);
}

void Basket::Release_Basket()
{
    m_RequestAccountID->Release();

    while (!m_dequeItems.empty()) {
        BasketItem* pItem = m_dequeItems.front();
        m_dequeItems.pop_front();
        delete pItem;
    }

    m_nSubCount = 0;
    m_lMinimumTransfer = 0;
    m_nTransferMultiple = 0;
    m_bHideAccountID = false;
    m_bExchangingIn = false;
    m_lClosingTransactionNo = 0;
}

void Basket::Release()
{
    Release_Basket();

    Contract::Release();
}

Basket::~Basket() { Release_Basket(); }
}  // namespace opentxs
