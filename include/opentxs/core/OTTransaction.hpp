// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_OTTRANSACTION_HPP
#define OPENTXS_CORE_OTTRANSACTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{

namespace api
{
namespace implementation
{

class Factory;

}  // namespace implementation
}  // namespace api

/*
WHEN THE server receives a transaction request, it receives a MESSAGE containing
an ascii-armored LEDGER.

 The TYPE of the message might be "process inbox" or "process these transfers".
 but either way there is a ledger bundled that contains a list of transactions
(often a list of one.)

 a ledger is stored as my inbox

 a ledger is sent in a message to send me my inbox

 a ledger is what I send the server when I ask it to process a couple of
transactions.
 Each one of them has a transaction number.
 Therefore the ledger must have a MAP of transactions, indexed by TRANSACTION
NUMBER.

 Therefore message cannot handle transaction number.

 Perhaps ledger is derived from message.  CMD3 is a ledger in an envelope.
 Then it can do everything that a message can do,




 A message contains a payload of a ledger

 the ledger contains a list of transactions

Ledger is derived from contract because you must be able to save / sign it and
load from string,
 and because it must have items in it.


 transactions are derived from messages. a transaction is a form of message.
 (technically you could do a CMD3 and just send a transaction but the above is
cleaner.)

 Messages already have server ID, command, Account ID,

 No a transaction is just a sibling to a message. It's it's own thing derived
from contract.
  but they have similarities

 so a ledger has a list of transactions. BOTH are derived from contract.

 A transaction has a list of Items. a transaction can also be "in reference to"
an item.

 does an item need to be a contract?

 each item has to be individually signed (because when I accept Sue's spend,
that signed accepted item goes back to Sue.)

 So the item has to be a contract. Each is signed and then put on the
transaction's list of items.

 Each transaction is also signed and includes a transaction number and is on the
ledger's list of transactions.

 The ledger itself is signed because it may be saved into a file.

 The whole enchilada is wrapped up in ascii-armor and attached to a message and
sent over the pipe.
*/

/*

 Partially loading transactions!  (New development, born out of necessity.)

 A few volunteers have played with OT test servers, and we (unsurprisingly)
started noticing messaging
 delays where a user had accumulated a thousand receipts in his inbox, and when
he tried to download it
 he would miss the message, and thus get out of sync. He can't download his
inbox anymore.

 So now, the inbox is not going to contain a copy of the actual receipts
themselves, as normally with
 transactions inside a ledger. Instead, the receipts will be saved into a
separate location, and they
 will be given a "save to ledger" function that will save the full thing (for
message ledgers) or a partial
 version (for inbox/outbox/nymbox ledgers) with no attachments (MUCH smaller
size) and which contains a hash
 of the actual full receipt.

 OT should just be able to ASSUME that the client software has downloaded the
relevant receipts (a message
 will be added to the protocol for downloading these receipts.) The transaction
object on an inbox will have
 a flag set to false until the actual receipt has been loaded (not just the
inbox placeholder) and certain
 actions will simply fail if that flag has not been set to true from the ACTUAL
full receipt being loaded.
 Or the function will automatically try to load said receipt, and then fail if
that action fails. If the receipt
 hash appears in the inbox, therefore the API should be able to assume that the
receipt itself is available for
 inspection.

 Todo:
 -- "Save To Abbreviated form" function.

 And what does it save?
 transactionType        m_Type;        // blank, pending, processInbox,
transfer, deposit, withdrawal, trade, etc.
 time64_t                    m_DATE_SIGNED;        // The date, in seconds, when
the instrument was last signed.
 std::int64_t                    m_lTransactionNum;    // The server issues this
and
it must be sent with transaction request.
 std::int64_t                    m_lInReferenceToTransaction;
 std::int64_t                    m_lClosingTransactionNo; // used by
finalReceipt
 also:                AMOUNT.  // GetReceiptAmount()
 std::int64_t                m_lAbbrevAmount; // Stored here after loading, but
not
saved from here in the first place (see GetReceiptAmount())
 std::int64_t                m_lDisplayAmount; // Just like m_lAbbrevAmount,
except
it stores the display amount. For example, a transferReceipt for a 5000 clam
transfer has an effective value of 0 (since the transfer is already done) but it
has a display amount of 5000.
 OTIdentifier        m_Hash;             // Created while saving abbreviated
record, loaded back with it, then verified against actual hash when loading
actual box receipt.

 DOES NOT SAVE:
 listOfItems    m_listItems;        // the various items in this transaction.
 Armored   m_ascCancellationRequest; // used by finalReceipt
//    OTIdentifier    m_ID;            // Account ID. This is in Contract
(parent class). Here we use it for the REAL ACCOUNT ID (set before loading.)
 OTIdentifier    m_AcctID;        // Compare m_AcctID to m_ID after loading it
from string or file. They should match, and signature should verify.
 OTIdentifier    m_NotaryID;        // Notary ID as used to instantiate the
transaction, based on expected NotaryID.
 OTIdentifier    m_AcctNotaryID;    // Actual NotaryID within the signed
portion. (Compare to m_NotaryID upon loading.)
 OTIdentifier    m_AcctNymID;        // NymID of the user who created this
item. (In the future, this item
 Armored    m_ascInReferenceTo;    // This item may be in reference to a
different item

 Normally we only save the "purported" values. But in "save to ledger" function
I only want to save the values
 that I'm EXPECTING to be in those receipts, not the values that are ACTUALLY in
those receipts. (Of course they
 ARE the values that were in the receipts when first saved, but I don't know
what will be in the receipts when I LOAD
 them again, until I load them and verify that they are still the right ones.
Perhaps a substitution occured in the meantime.

 In any case, when I am making the abbreviated record, I already know that
information from the ledger itself
 (user ID, account ID, server ID, etc) so I don't bother storing it for each
transaction at all since it's already
 stored for the ledger as a whole. I can set the expected numbers based on that,
and then verify it against the purported
 numbers when the ACTUAL receipt is loaded up.

 Hmm ProduceOutboxReportItem() and ProduceInboxReportItem() ALREADY do the work
of grabbing the Type, transaction#,
 the reference #, the closing #, and the amount. Therefore I'll use that to do
the work, and then iterate the balance
 item's sub-items when saving in abbreviated form. But this means I have to
store the hash in the sub-items (at the time
 of their creation) which means I probably want to just keep it there for the
normal balance agreements as well. But
 for normal balance agreements, the sub-items are stored on an actual item, so
I'll check out that code... maybe just save
 that item itself directly in the ledger... except the balance portions aren't
appropriate, only the sub-item portions.
 CHANGE OF HEART:  I won't be using the sub-items for this, the difference is
great enough that I'd be forcing a square
 peg into a round hole. Just adding a few functions for saving/loading in
abbreviated form.


 NEED TO CODE:
 -- Server message for downloading "box receipts."

 -- Method on OTTransaction for saving in abbreviated format to a string. Called
by...

 -- Fix OTLedger Load/Save so it only loads/saves transactions in abbreviated
format, which should only be
    useful for getting enough information in order to download the receipts.
From there, the existing "verify"
    function should try to load them all before actually verifying. (From there,
you have loaded and verified the transaction,
    which we normally do anyway before using it, so now that's all done, so we
can go ahead and use it as normal.)

 -- How does user get his box receipts? downloads from server and loads during
verify process.

 -- How does Ledger save? It calls OTTransaction::SaveToAbbreviatedForm() or
some such, in a loop, and saves all their
    vitals.

 -- How about the transactions themselves (the full "box receipts" and not just
abbreviated versions) ?
    1. User never saves them other than when downloading from server, and he
must compare them against the hash in the box.
    2. User loads them as part of verify process when verifying
inbox/outbox/nymbox.
    3. Server must First Save them at the same time that they are added to the
box itself.
    4. From there server can load anytime also, similar to user (verify
process.) Server should never fail since it CREATED the receipt.

 -- Server is safe to erase the box receipt at the same time that it's been
removed from its box. In practice this
    should be handled by saving the box receipt with a MARKED FOR DELETION flag
added.

 -- WHERE to save? (client and server side)

 Keeping in mind there are also "cron receipts" (OTCronItem is for recurring
transactions like market offers/trades,
 and smart contracts) as well as "transaction receipts" (*every* server reply to
every transaction request is automatically
 saved, success or fail, as a transaction receipt.) They are stored like so:

 ~/.ot/client_data/receipts/tmHvLZxb13hW6OyH1oHKTmKd7fTMRsUfzqPE6KCwSjl
 > ls
 1mxwHcQvxSgqWqy2e28ZREeuVVfgs0kxE41FN10TnS3.success
2K3LoD1UxsJLO0FAfng0fgqnxnXScO8rM2eO5tdDTOE.success
T1Q3wZWgeTUoaUvn9m1lzIK5tn5wITlzxzrGNI8qtaV.success
vuXpQt8h6CRGcz6G5zMOXqQeIRpsOq05VLuJ5IAFS5R.success
 2FHFr5NdT1r1XtUWVjll1uGcTKGKx4Pt1iWJ9eV0kkZ.success
A6CcGwNiTtEPQnv7HLUcmf0QFaCGy862pb1SJ3zVqIU.success
gys4puOdx15pknQYcVtF4DmOo2tOYouoiqcQAHKApSF.success
w06QIURsSDV7sdWWvyPSxRaiv4T2MUQbJyWmIKxSmuL.success

 basically:
 OT_DATA/receipts/NOTARY_ID/ACCT_ID.success
 OT_DATA/receipts/NOTARY_ID/ACCT_ID.fail

 or:
 OT_DATA/receipts/NOTARY_ID/NYM_ID.success
 OT_DATA/receipts/NOTARY_ID/NYM_ID.fail

 These receipts are related to balance agreement. These are the "last signed
receipt" that you keep in order to
 verify later against future intermediary files, to make sure the server isn't
screwing you with bad intermediary
 files. Sometimes there is a transaction statement, and sometimes there is a
balance agreement (depending on whether
 both sides are merely agreeing on the list of signed-out transaction numbers,
or whether they are additionally
 agreement on the balance and inbox items for a specific asset account as well.)

 (The transaction statements are the ones stored by NYM_ID, and the balance
agreements are the ones by ACCT_ID.)

 -----------

 In addition to "cron receipts" and "transaction receipts", I am now adding this
idea of "box receipts" which is really
 just your inbox items stored in a separate file, because they simply won't all
fit comfortably in a getInbox message.
 How to store them?

 Current inbox/outbox path:    OT_DATA/[inbox|outbox]/NOTARY_ID/ACCT_ID
 Current nymbox path:        OT_DATA/nymbox/NOTARY_ID/NYM_ID

 Therefore I propose the box receipts to be stored:

 Inbox/outbox path:    OT_DATA/inbox/NOTARY_ID/ACCT_ID.r/TRANSACTION_ID.rct
 nymbox path:        OT_DATA/nymbox/NOTARY_ID/NYM_ID.r/TRANSACTION_ID.rct

 When querying the server for a box receipt, you will have to provide TYPE
(inbox, outbox, etc)
 as well as ID (acct ID or Nym ID, depending on which box type) as well as
transaction number,
 and the hash.

 ------------

 Note: When loading from string, IF IN ABBREVIATED FORM, then we must take care
to verify the
 loaded values against their expected counterparts, before OVERWRITING the
"expected" values
 with the actual ones from the box receipts. Rather than store each value
separately, (as we did
 with notaryID, acctID, etc in OTTransactionType) we will instead simply take
care to verify them
 during loading, if the "abbreviated" flag is set.

 ------------

 THE BEST TIME to actually SAVE the box receipt, on the Server Side, is at the
same time that it is first being
 added to the relevant box itself. (Look into centralizing that mechanism...)

 */
class OTTransaction : public OTTransactionType
{
public:
    // a transaction can be blank (issued from server)
    // or pending (in the inbox/outbox)
    // or it can be a "process inbox" transaction
    // might also be in the nymbox.
    // See transactionType in Types.hpp.

    void Release() override;
    EXPORT std::int64_t GetNumberOfOrigin() override;
    EXPORT void CalculateNumberOfOrigin() override;

    // This calls VerifyContractID() as well as VerifySignature()
    // Use this instead of Contract::VerifyContract, which expects/uses a
    // pubkey from inside the contract.
    bool VerifyAccount(const Nym& theNym) override;

    void InitTransaction();

    EXPORT bool IsCancelled() { return m_bCancelled; }

    EXPORT void SetAsCancelled() { m_bCancelled = true; }

    void SetParent(const Ledger& theParent) { m_pParent = &theParent; }

    EXPORT bool AddNumbersToTransaction(const NumList& theAddition);

    bool IsAbbreviated() const { return m_bIsAbbreviated; }

    std::int64_t GetAbbrevAdjustment() const { return m_lAbbrevAmount; }

    void SetAbbrevAdjustment(std::int64_t lAmount)
    {
        m_lAbbrevAmount = lAmount;
    }

    std::int64_t GetAbbrevDisplayAmount() const { return m_lDisplayAmount; }

    void SetAbbrevDisplayAmount(std::int64_t lAmount)
    {
        m_lDisplayAmount = lAmount;
    }

    std::int64_t GetAbbrevInRefDisplay() const { return m_lInRefDisplay; }

    void SetAbbrevInRefDisplay(std::int64_t lVal) { m_lInRefDisplay = lVal; }

    // These are used exclusively by replyNotice (so you can tell
    // which reply message it's a notice of.)
    const std::int64_t& GetRequestNum() const { return m_lRequestNumber; }

    void SetRequestNum(const std::int64_t& lNum) { m_lRequestNumber = lNum; }

    bool GetReplyTransSuccess() { return m_bReplyTransSuccess; }

    void SetReplyTransSuccess(bool bVal) { m_bReplyTransSuccess = bVal; }

    // These are used for finalReceipt and basketReceipt
    EXPORT std::int64_t GetClosingNum() const;
    EXPORT void SetClosingNum(std::int64_t lClosingNum);
    EXPORT std::int64_t GetReferenceNumForDisplay();  /// For display purposes.
                                                      /// The
    /// "ref #" you actually display
    /// (versus the one you use
    /// internally) might change
    /// based on transaction type.
    /// (Like with a cheque receipt
    /// you actually have to load up
    /// the original cheque.)

    EXPORT bool GetSenderNymIDForDisplay(identifier::Nym& theReturnID);
    EXPORT bool GetRecipientNymIDForDisplay(identifier::Nym& theReturnID);

    EXPORT bool GetSenderAcctIDForDisplay(Identifier& theReturnID);
    EXPORT bool GetRecipientAcctIDForDisplay(Identifier& theReturnID);
    EXPORT bool GetMemo(String& strMemo);

    inline time64_t GetDateSigned() const { return m_DATE_SIGNED; }

    // Tries to determine, based on items within,
    // whether the transaction was a success or fail.
    // NOTE: The 2 parameters are for notices. A notice
    // is not a "transaction" per se, but it is used to
    // give notice that a cron item has successfully activated
    // (or not.) So GetSuccess will return false if it's a notice,
    // so that's why we have the 2 bool parameters.
    //
    // NOTE: I'm also using the 2 parameters for regular success status,
    // not just notice success status. Why? Because the boolean returned
    // by this function, if false, doesn't mean there was a failure. Rather,
    // it means there was no transaction at all, OR there was, but it failed.
    // Much of the code uses it this way, not needing any more detail than that.
    // But certain code may someday really need to know the nitty-gritty,
    // so I'm using these paramters to return that information.
    //
    // WARNING: returns FALSE for notices and receipts, even if the 2 parameters
    // come back as TRUE. That's because it ONLY returns TRUE in the event that
    // this
    // transaction is an actual transaction. (And a receipt or notice is not an
    // actual
    // transaction.)
    EXPORT bool GetSuccess(
        bool* pbHasSuccess = nullptr,
        bool* pbIsSuccess = nullptr);

    EXPORT std::int64_t GetReceiptAmount();  // Tries to determine IF there is
                                             // an
    // amount (depending on type) and return
    // it.

    transactionType GetType() const;
    void SetType(transactionType theType);

    // This function assumes that theLedger is the owner of this transaction.
    // We pass the ledger in so we can determine the proper directory we're
    // reading from.
    EXPORT bool SaveBoxReceipt(std::int64_t lLedgerType);

    EXPORT bool SaveBoxReceipt(Ledger& theLedger);

    EXPORT bool DeleteBoxReceipt(Ledger& theLedger);

    // Call on abbreviated version, and pass in the purported full version.
    bool VerifyBoxReceipt(OTTransaction& theFullVersion);

    EXPORT bool VerifyBalanceReceipt(const ServerContext& context);

    // First VerifyContractID() is performed already on all the items when
    // they are first loaded up. NotaryID and AccountID have been verified.
    // Now we check ownership, and signatures, and transaction #s, etc.
    // (We go deeper.)
    EXPORT bool VerifyItems(const Nym& theNym);

    inline std::int32_t GetItemCount() const
    {
        return static_cast<std::int32_t>(m_listItems.size());
    }

    std::int32_t GetItemCountInRefTo(std::int64_t lReference);  // Count the
                                                                // number
    // of items that are
    // IN REFERENCE TO
    // some transaction#.

    // While processing a transaction, you may wish to query it for items of a
    // certain type.
    EXPORT std::shared_ptr<Item> GetItem(itemType theType);

    EXPORT std::shared_ptr<Item> GetItemInRefTo(std::int64_t lReference);

    EXPORT void AddItem(std::shared_ptr<Item> theItem);  // You have to allocate
                                                         // the item on the heap
                                                         // and then pass it in
                                                         // as a reference.
    // OTTransaction will take care of it from there and will delete it in
    // destructor.
    // used for looping through the items in a few places.
    inline listOfItems& GetItemList() { return m_listItems; }

    // Because all of the actual receipts cannot fit into the single inbox
    // file, you must put their hash, and then store the receipt itself
    // separately...
    void SaveAbbreviatedNymboxRecord(Tag& parent);
    void SaveAbbreviatedOutboxRecord(Tag& parent);
    void SaveAbbreviatedInboxRecord(Tag& parent);
    void SaveAbbrevPaymentInboxRecord(Tag& parent);
    void SaveAbbrevRecordBoxRecord(Tag& parent);
    void SaveAbbrevExpiredBoxRecord(Tag& parent);
    void ProduceInboxReportItem(Item& theBalanceItem);
    void ProduceOutboxReportItem(Item& theBalanceItem);

    static transactionType GetTypeFromString(const String& strType);

    const char* GetTypeString() const;

    // These functions are fairly smart about which transaction types are
    // harvestable,
    // in which situations (based on the bools.) As long as you use the bools
    // correctly,
    // you aren't likely to accidentally harvest an opening number unless you
    // are SUPPOSED
    // to harvest it, based on its type and the circumstances. Just make sure
    // you are accurate
    // when you tell it the circumstances (bools!)
    bool HarvestOpeningNumber(
        ServerContext& context,
        bool bHarvestingForRetry,      // exchangeBasket, on retry, needs to
                                       // clawback the opening # because it
                                       // will be using another opening # the
                                       // next time OT_API_exchangeBasket() is
                                       // called.
        bool bReplyWasSuccess,         // false until positively asserted.
        bool bReplyWasFailure,         // false until positively asserted.
        bool bTransactionWasSuccess,   // false until positively asserted.
        bool bTransactionWasFailure);  // false until positively asserted.

    // NOTE: IN CASE it's not obvious, the NYM is harvesting numbers from the
    // TRANSACTION, and not the other way around!

    // Normally do this if your transaction ran--and failed--so you can get most
    // of your transaction numbers back. (The opening number is already gone,
    // but any others are still salvageable.)
    bool HarvestClosingNumbers(
        ServerContext& context,
        bool bHarvestingForRetry,      // exchangeBasket, on retry, needs to
                                       // clawback the opening # because it
                                       // will be using another opening # the
                                       // next time OT_API_exchangeBasket() is
                                       // called.
        bool bReplyWasSuccess,         // false until positively asserted.
        bool bReplyWasFailure,         // false until positively asserted.
        bool bTransactionWasSuccess,   // false until positively asserted.
        bool bTransactionWasFailure);  // false until positively asserted.

    OTIdentifier GetAccountHash() const { return m_accounthash; }
    OTIdentifier GetInboxHash() const { return m_inboxhash; }
    OTIdentifier GetOutboxHash() const { return m_outboxhash; }
    void SetAccountHash(const Identifier& accounthash)
    {
        m_accounthash = accounthash;
    }
    void SetInboxHash(const Identifier& inboxhash) { m_inboxhash = inboxhash; }
    void SetOutboxHash(const Identifier& outboxhash)
    {
        m_outboxhash = outboxhash;
    }

    EXPORT virtual ~OTTransaction();

protected:
    // Usually a transaction object is inside a ledger object.
    // If this is not nullptr, then you can reference that object.
    const Ledger* m_pParent{nullptr};

    // Transactions can be loaded in abbreviated form from a ledger, but they
    // are not considered "actually loaded"
    // until their associated "box receipt" is also loaded up from storage, and
    // verified against its hash.
    // From the time they are loaded in abbreviated form, this flag is set true,
    // until the box receipt is loaded.
    // This value defaults to false, so if the transaction was never loaded in
    // abbreviated form, then this is never
    // set to true in the first place.
    bool m_bIsAbbreviated{false};

    // The "Amount" of the transaction is not normally stored in the transaction
    // itself, but in one of its
    // transaction items. However, when saving/loading the transaction in
    // abbreviated form, the amount is
    // placed here, which makes it available for necessary calculations without
    // being forced to load up
    // all of the box receipts to do so.

    std::int64_t m_lAbbrevAmount{0};

    // Just like m_lAbbrevAmount, except it stores the display amount. For
    // example, a transferReceipt for
    // a 5000 clam transfer has an effective value of 0 (since the transfer is
    // already done) but it has a
    // display amount of 5000.
    // As with m_lAbbrevAmount, the Display amount value is calculated just
    // before saving in abbreviated
    // form, and this variable is only set upon LOADING that value in
    // abbreviated form. The actual value
    // only becomes available when loading the ACTUAL box receipt, at which time
    // it should be compared to
    // this one. (If loading the transaction fails, as a result of a failed
    // verification there, then these
    // numbers become pretty reliable and can be used in optimization, since the
    // current process of loading
    // transaction items from a string every time we need to check the amount,
    // can be time-consuming, CPU-wise.)
    std::int64_t m_lDisplayAmount{0};

    // The value of GetReferenceNumForDisplay() is saved when saving an
    // abbreviated record of this transaction,
    // and then loaded into THIS member variable when loading the abbreviated
    // record.
    std::int64_t m_lInRefDisplay{0};

    // This hash is not stored inside the box receipt itself (a transaction that
    // appears in an inbox, outbox, or nymbox)
    // but rather, is set from above, and then verified against the actual box
    // receipt once it is loaded.
    // This verification occurs only when loading the box receipt, and this Hash
    // value is not saved again to any
    // location. When the abbreviated form of the box receipt is saved inside
    // the inbox itself, it is easy to
    // just create the hash at that time. Then upon loading the ledger (the box)
    // the hash is set based on the abbreviated entry.
    // The hash can then be verified against the actual box receipt by hashing
    // that and comparing them, after which I no
    // longer care about this variable at all, and do not save it again, since
    // it can be re-calculated the next time we
    // save again in abbreviated form.
    OTIdentifier m_Hash;  // todo: make this const and force it to be set during
                          // construction.

    time64_t m_DATE_SIGNED{0};  // The date, in seconds, when the instrument was
                                // last signed.
    transactionType m_Type{
        transactionType::error_state};  // blank, pending, processInbox,
                                        // transfer, deposit, withdrawal,
                                        // trade, etc.
    listOfItems m_listItems;  // the various items in this transaction.

    TransactionNumber m_lClosingTransactionNo{0};  // used by finalReceipt
    OTArmored m_ascCancellationRequest;            // used by finalReceipt

    // ONLY the "replyNotice" transaction uses this field.
    // When replyNotices are dropped into your Nymbox (server notices
    // of replies to messages you sent in the past) I wanted to put that
    // they were "in reference to" a specific request number. But I don't
    // want to muddy the "in reference to" code now which currently only
    // references transaction numbers, not request numbers. Therefore I
    // have added a special variable here for request numbers, so that
    // replyNotices in the Nymbox can directly finger the messages they
    // came from.
    RequestNumber m_lRequestNumber{0};  // Unused except by "replyNotice" in
                                        // Nymbox.
    bool m_bReplyTransSuccess{false};   // Used only by replyNotice
    // Unused except for notarizeTransactionResponse, specifically for
    // @paymentPlan
    // and @smartContract. (And maybe @depositCheque...) There are specific
    // cases where the user sends through a transaction that is MEANT to be
    // rejected by the server, for the purpose of cancelling that transaction.
    // For example, if I sent a smart contract on to the next party, and then
    // later I decided to cancel it (before the next party had the chance to
    // activate it.) I send through the (incomplete) contract AS THOUGH to
    // activate it, specifically so the activation will fail (and thus cancel
    // the smart contract.) This prevents anyone from activating it in the
    // future (thus, it's now "cancelled.")
    //
    // In these cases, when I am "cancelling" something, a successful
    // cancellation
    // will result in a "rejected" reply from the server. A notice of this is
    // sent to all the parties as well. The rejection notice causes all the
    // parties to properly harvest their transaction numbers as they normally
    // would when a smart contract fails activation. (Thus, a successful
    // cancellation.)
    //
    // But how do we know the difference between a normal "rejection" versus
    // a "rejection" that corresponds to a successful cancellation? That's
    // what m_bCancelled is for. If the server has just successfully cancelled
    // something, it will set m_bCancelled to TRUE (on the reply transaction.)
    // This way the client side can tell the difference between an actual
    // failed attempt marked as "rejected", versus a successful cancellation
    // marked as "rejected." All the client has to do is check m_bCancelled
    // to see if it's set to TRUE, and it will know.
    bool m_bCancelled{false};

    OTIdentifier m_inboxhash;
    OTIdentifier m_outboxhash;
    OTIdentifier m_accounthash;

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    void UpdateContents() override;  // Before transmission or serialization,
                                     // this is where the transaction saves its
                                     // contents

private:
    friend api::implementation::Factory;

    OTTransaction(const api::Core& core);
    OTTransaction(const api::Core& core, const Ledger& theOwner);

    OTTransaction(
        const api::Core& core,
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        originType theOriginType = originType::not_applicable);

    OTTransaction(
        const api::Core& core,
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        std::int64_t lTransactionNum,
        originType theOriginType = originType::not_applicable);

    // THIS constructor only used when loading an abbreviated box receipt
    // (inbox, nymbox, or outbox receipt).
    // The full receipt is loaded only after the abbreviated ones are loaded,
    // and verified against them.
    OTTransaction(
        const api::Core& core,
        const identifier::Nym& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID,
        const std::int64_t& lNumberOfOrigin,
        originType theOriginType,
        const std::int64_t& lTransactionNum,
        const std::int64_t& lInRefTo,
        const std::int64_t& lInRefDisplay,
        time64_t the_DATE_SIGNED,
        transactionType theType,
        const String& strHash,
        const std::int64_t& lAdjustment,
        const std::int64_t& lDisplayValue,
        const std::int64_t& lClosingNum,
        const std::int64_t& lRequestNum,
        bool bReplyTransSuccess,
        NumList* pNumList = nullptr);

    OTTransaction() = delete;
};
}  // namespace opentxs
#endif
