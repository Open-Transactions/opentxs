// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>

#include "internal/otx/Types.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
namespace imp
{
class Factory;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace otx
{
namespace context
{
class Client;
}  // namespace context
}  // namespace otx

class Account;
class Data;
class Item;
class Ledger;
class NumList;
class OTTransaction;
class PasswordPrompt;
class String;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using listOfItems = UnallocatedList<std::shared_ptr<Item>>;

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
    auto AddBlankNumbersToItem(const NumList& theAddition) -> bool;
    auto GetClosingNum() const -> std::int64_t;
    void SetClosingNum(std::int64_t lClosingNum);
    auto GetNumberOfOrigin() -> std::int64_t override;
    void CalculateNumberOfOrigin() override;
    // used for looping through the items in a few places.
    inline auto GetItemList() -> listOfItems& { return list_items_; }
    auto GetItem(std::int32_t nIndex) const -> std::shared_ptr<const Item>;
    auto GetItem(std::int32_t nIndex) -> std::shared_ptr<Item>;
    auto GetItemByTransactionNum(std::int64_t lTransactionNumber)
        -> std::shared_ptr<Item>;  // While
    // processing
    // an item, you
    // may
    // wish to query it for sub-items
    auto GetFinalReceiptItemByReferenceNum(std::int64_t lReferenceNumber)
        -> std::shared_ptr<Item>;  // The final receipt item MAY be
                                   // present, and co-relates to others
                                   // that share its "in reference to"
                                   // value. (Others such as
                                   // marketReceipts and paymentReceipts.)
    auto GetItemCountInRefTo(std::int64_t lReference)
        -> std::int32_t;  // Count the
                          // number
    // of items that are
    // IN REFERENCE TO
    // some transaction#.
    inline auto GetItemCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(list_items_.size());
    }
    void AddItem(std::shared_ptr<Item> theItem);

    void ReleaseItems();
    void Release_Item();
    void Release() override;
    // the "From" accountID and the NotaryID are now in the parent class. (2 of
    // each.)

    inline void SetNewOutboxTransNum(std::int64_t lTransNum)
    {
        new_outbox_trans_num_ = lTransNum;
    }
    inline auto GetNewOutboxTransNum() const -> std::int64_t
    {
        return new_outbox_trans_num_;
    }                 // See above comment in protected section.
    OTArmored note_;  // a text field for the user. Cron may also store
    // receipt data here. Also inbox reports go here for
    // balance agreement
    OTArmored attachment_;  // the digital cash token is sent here,
                            // signed, and returned here. (or purse of
                            // tokens.)
    // As well as a cheque, or a voucher, or a server update on a market offer,
    // or a nym full of transactions for balance agreement.
    // Call this on the server side, on a balanceStatement item, to verify
    // whether the wallet side set it up correctly (and thus it's okay to sign
    // and return with acknowledgement.)
    auto VerifyBalanceStatement(
        const Amount& lActualAdjustment,
        const otx::context::Client& context,
        const Ledger& THE_INBOX,
        const Ledger& THE_OUTBOX,
        const Account& THE_ACCOUNT,
        const OTTransaction& TARGET_TRANSACTION,
        const UnallocatedSet<TransactionNumber>& excluded,
        const PasswordPrompt& reason,
        TransactionNumber outboxNum = 0) const
        -> bool;  // Used in special case of
                  // transfers (the user didn't
                  // know the outbox trans# when
                  // constructing the original
                  // request.) Unused when 0.
                  // server-side
    auto VerifyTransactionStatement(
        const otx::context::Client& THE_NYM,
        const OTTransaction& TARGET_TRANSACTION,
        const UnallocatedSet<TransactionNumber> newNumbers,
        const bool bIsRealTransaction = true) const -> bool;
    auto VerifyTransactionStatement(
        const otx::context::Client& THE_NYM,
        const OTTransaction& TARGET_TRANSACTION,
        const bool bIsRealTransaction = true) const -> bool;
    inline auto GetStatus() const -> Item::itemStatus { return status_; }
    inline void SetStatus(const Item::itemStatus& theVal) { status_ = theVal; }
    inline auto GetType() const -> itemType { return type_; }
    inline void SetType(itemType theType) { type_ = theType; }
    inline auto GetAmount() const -> Amount { return amount_; }
    inline void SetAmount(Amount lAmount) { amount_ = lAmount; }
    void GetNote(String& theStr) const;
    void SetNote(const String& theStr);
    void GetAttachment(String& theStr) const;
    void GetAttachment(Data& output) const;
    void SetAttachment(const String& theStr);
    void SetAttachment(const Data& input);
    inline auto GetDestinationAcctID() const -> const identifier::Generic&
    {
        return account_to_id_;
    }
    inline void SetDestinationAcctID(const identifier::Generic& theID)
    {
        account_to_id_ = theID;
    }
    static void GetStringFromType(itemType theType, String& strType);
    inline void GetTypeString(String& strType) const
    {
        GetStringFromType(GetType(), strType);
    }
    void InitItem();

    Item() = delete;

    ~Item() override;

protected:
    // DESTINATION ACCOUNT for transfers. NOT the account holder.
    identifier::Generic account_to_id_;
    // For balance, or fee, etc. Only an item can actually have an amount. (Or a
    // "TO" account.)
    Amount amount_{0};
    // Sometimes an item needs to have a list of yet more items. Like balance
    // statements have a list of inbox items. (Just the relevant data, not all
    // the attachments and everything.)
    listOfItems list_items_;
    // the item type. Could be a transfer, a fee, a balance or client
    // accept/rejecting an item
    itemType type_{itemType::error_state};
    // request, acknowledgment, or rejection.
    itemStatus status_{error_status};
    // Used for balance agreement. The user puts transaction "1" in his outbox
    // when doing a transfer, since he has no idea what # will actually be
    // issued on the server side after he sends his message. Let's say the
    // server issues # 34, and puts that in the outbox. It thus sets this member
    // to 34, and it is understood that 1 in the client request corresponds to
    // 34 on this member variable in the reply.  Only one transfer can be done
    // at a time. In cases where verifying a balance receipt and you come across
    // transaction #1 in the outbox, simply look up this variable on the
    // server's portion of the reply and then look up that number instead.
    TransactionNumber new_outbox_trans_num_{0};
    // Used in balance agreement (to represent an inbox item)
    TransactionNumber closing_transaction_no_{0};

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;
    // Before transmission or serialization, this is where the ledger saves its
    // contents
    void UpdateContents(const PasswordPrompt& reason) override;

private:  // Private prevents erroneous use by other classes.
    friend api::session::imp::Factory;

    using ot_super = OTTransactionType;

    auto GetItemTypeFromString(const String& strType) -> itemType;

    // There is the OTTransaction transfer, which is a transaction type, and
    // there is also the Item transfer, which is an item type. They are
    // related. Every transaction has a list of items, and these perform the
    // transaction. A transaction trying to TRANSFER would have these items:
    // transfer, serverfee, balance, and possibly outboxhash.
    Item(const api::Session& api);
    Item(
        const api::Session& api,
        const identifier::Nym& theNymID,
        const Item& theOwner);  // From owner we can get acct ID, server ID,
                                // and transaction Num
    Item(
        const api::Session& api,
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner);  // From owner we can get acct ID,
                                         // server ID, and transaction Num
    Item(
        const api::Session& api,
        const identifier::Nym& theNymID,
        const OTTransaction& theOwner,
        itemType theType,
        const identifier::Generic& pDestinationAcctID);
};
}  // namespace opentxs
