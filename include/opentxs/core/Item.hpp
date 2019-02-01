// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_ITEM_HPP
#define OPENTXS_CORE_ITEM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <list>

namespace opentxs
{
namespace api
{
namespace implementation
{

class Factory;

}  // namespace implementation
}  // namespace api

typedef std::list<std::shared_ptr<Item>> listOfItems;

// Item as in "Transaction Item"
// An OTLedger contains a list of transactions (pending transactions, inbox or
// outbox.)
// Each transaction has a list of items that make up that transaction.
// I think that the Item ID shall be the order in which the items are meant to
// be processed.
// Items are like tracks on a CD. It is assumed there will be several of them,
// they
// come in packs. You normally would deal with the transaction as a single
// entity,
// not the item. A transaction contains a list of items.
class Item : public OTTransactionType
{
public:
    // FOR EXAMPLE:  A client may send a TRANSFER request, setting type to
    // Transfer and status to Request.
    //                 The server may respond with type atTransfer and status
    // Acknowledgment.
    //                            Make sense?

    enum itemStatus {
        request,          // This item is a request from the client
        acknowledgement,  // This item is an acknowledgment from the server.
                          // (The
                          // server has signed it.)
        rejection,    // This item represents a rejection of the request by the
                      // server. (Server will not sign it.)
        error_status  // error status versus error state
    };

    // For "Item::acceptTransaction" -- the blank contains a list of blank
    // numbers,
    // therefore the "accept" must contain the same list. Otherwise you haven't
    // signed off!!
    //
    //
    EXPORT bool AddBlankNumbersToItem(const NumList& theAddition);
    std::int64_t GetClosingNum() const;
    void SetClosingNum(std::int64_t lClosingNum);
    EXPORT std::int64_t GetNumberOfOrigin() override;
    EXPORT void CalculateNumberOfOrigin() override;
    // used for looping through the items in a few places.
    inline listOfItems& GetItemList() { return m_listItems; }
    std::shared_ptr<const Item> GetItem(std::int32_t nIndex) const;
    std::shared_ptr<Item> GetItem(std::int32_t nIndex);
    std::shared_ptr<Item> GetItemByTransactionNum(
        std::int64_t lTransactionNumber);  // While
    // processing
    // an item, you
    // may
    // wish to query it for sub-items
    std::shared_ptr<Item> GetFinalReceiptItemByReferenceNum(
        std::int64_t lReferenceNumber);  // The final receipt item MAY be
                                         // present, and co-relates to others
                                         // that share its "in reference to"
                                         // value. (Others such as
                                         // marketReceipts and paymentReceipts.)
    std::int32_t GetItemCountInRefTo(std::int64_t lReference);  // Count the
                                                                // number
    // of items that are
    // IN REFERENCE TO
    // some transaction#.
    inline std::int32_t GetItemCount() const
    {
        return static_cast<std::int32_t>(m_listItems.size());
    }
    void AddItem(std::shared_ptr<Item> theItem);

    void ReleaseItems();
    void Release_Item();
    void Release() override;
    // the "From" accountID and the NotaryID are now in the parent class. (2 of
    // each.)

    inline void SetNewOutboxTransNum(std::int64_t lTransNum)
    {
        m_lNewOutboxTransNum = lTransNum;
    }
    inline std::int64_t GetNewOutboxTransNum() const
    {
        return m_lNewOutboxTransNum;
    }                     // See above comment in protected section.
    OTArmored m_ascNote;  // a text field for the user. Cron may also store
    // receipt data here. Also inbox reports go here for
    // balance agreement
    OTArmored m_ascAttachment;  // the digital cash token is sent here,
                                // signed, and returned here. (or purse of
                                // tokens.)
    // As well as a cheque, or a voucher, or a server update on a market offer,
    // or a nym full of transactions for balance agreement.
    // Call this on the server side, on a balanceStatement item, to verify
    // whether the wallet side set it up correctly (and thus it's okay to sign
    // and return with acknowledgement.)
    EXPORT bool VerifyBalanceStatement(
        std::int64_t lActualAdjustment,
        const ClientContext& context,
        const Ledger& THE_INBOX,
        const Ledger& THE_OUTBOX,
        const Account& THE_ACCOUNT,
        const OTTransaction& TARGET_TRANSACTION,
        const std::set<TransactionNumber>& excluded,
        TransactionNumber outboxNum = 0) const;  // Used in special case of
                                                 // transfers (the user didn't
                                                 // know the outbox trans# when
                                                 // constructing the original
                                                 // request.) Unused when 0.
                                                 // server-side
    EXPORT bool VerifyTransactionStatement(
        const ClientContext& THE_NYM,
        const OTTransaction& TARGET_TRANSACTION,
        const std::set<TransactionNumber> newNumbers,
        const bool bIsRealTransaction = true) const;
    EXPORT bool VerifyTransactionStatement(
        const ClientContext& THE_NYM,
        const OTTransaction& TARGET_TRANSACTION,
        const bool bIsRealTransaction = true) const;
    inline Item::itemStatus GetStatus() const { return m_Status; }
    inline void SetStatus(const Item::itemStatus& theVal) { m_Status = theVal; }
    inline itemType GetType() const { return m_Type; }
    inline void SetType(itemType theType) { m_Type = theType; }
    inline std::int64_t GetAmount() const { return m_lAmount; }
    inline void SetAmount(std::int64_t lAmount) { m_lAmount = lAmount; }
    EXPORT void GetNote(String& theStr) const;
    EXPORT void SetNote(const String& theStr);
    EXPORT void GetAttachment(String& theStr) const;
    EXPORT void GetAttachment(Data& output) const;
    EXPORT void SetAttachment(const String& theStr);
    EXPORT void SetAttachment(const Data& input);
    inline const Identifier& GetDestinationAcctID() const { return m_AcctToID; }
    inline void SetDestinationAcctID(const Identifier& theID)
    {
        m_AcctToID = theID;
    }
    EXPORT static void GetStringFromType(itemType theType, String& strType);
    inline void GetTypeString(String& strType) const
    {
        GetStringFromType(GetType(), strType);
    }
    void InitItem();

    virtual ~Item();

protected:
    // DESTINATION ACCOUNT for transfers. NOT the account holder.
    OTIdentifier m_AcctToID;
    // For balance, or fee, etc. Only an item can actually have an amount. (Or a
    // "TO" account.)
    Amount m_lAmount{0};
    // Sometimes an item needs to have a list of yet more items. Like balance
    // statements have a list of inbox items. (Just the relevant data, not all
    // the attachments and everything.)
    listOfItems m_listItems;
    // the item type. Could be a transfer, a fee, a balance or client
    // accept/rejecting an item
    itemType m_Type{itemType::error_state};
    // request, acknowledgment, or rejection.
    itemStatus m_Status{error_status};
    // Used for balance agreement. The user puts transaction "1" in his outbox
    // when doing a transfer, since he has no idea what # will actually be
    // issued on the server side after he sends his message. Let's say the
    // server issues # 34, and puts that in the outbox. It thus sets this member
    // to 34, and it is understood that 1 in the client request corresponds to
    // 34 on this member variable in the reply.  Only one transfer can be done
    // at a time. In cases where verifying a balance receipt and you come across
    // transaction #1 in the outbox, simply look up this variable on the
    // server's portion of the reply and then look up that number instead.
    TransactionNumber m_lNewOutboxTransNum{0};
    // Used in balance agreement (to represent an inbox item)
    TransactionNumber m_lClosingTransactionNo{0};

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    // Before transmission or serialization, this is where the ledger saves its
    // contents
    void UpdateContents() override;

private:  // Private prevents erroneous use by other classes.
    friend api::implementation::Factory;

    typedef OTTransactionType ot_super;

    itemType GetItemTypeFromString(const String& strType);

    // There is the OTTransaction transfer, which is a transaction type, and
    // there is also the Item transfer, which is an item type. They are
    // related. Every transaction has a list of items, and these perform the
    // transaction. A transaction trying to TRANSFER would have these items:
    // transfer, serverfee, balance, and possibly outboxhash.
    Item(const api::Core& core);
    Item(
        const api::Core& core,
        const identifier::Nym& theNymID,
        const Item& theOwner);  // From owner we can get acct ID, server ID,
                                // and transaction Num
    Item(
        const api::Core& core,
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner);  // From owner we can get acct ID,
                                         // server ID, and transaction Num
    Item(
        const api::Core& core,
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const Identifier& pDestinationAcctID);

    Item() = delete;
};
}  // namespace opentxs
#endif
