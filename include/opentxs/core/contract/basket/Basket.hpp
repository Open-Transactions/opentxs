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

#ifndef OPENTXS_BASKET_BASKET_HPP
#define OPENTXS_BASKET_BASKET_HPP

#include "opentxs/core/contract/basket/BasketItem.hpp"
#include "opentxs/core/Contract.hpp"

/*

 I figured this one out, it's easy.

 Someone creates a contract that contains 10 sub-contracts. It just delegates
 the issuence to the sub-issuers.

 When he connects to the server he can upload the contract, but he has no
 control over it at that point,
 since he is not one of the real issuers.

 The contract will only work if the sub-issuers actually have issued currencies
 on that transaction server.

 Then, the transaction server itself becomes the "issuer" of the basket
 currency.  It simply creates an issuer
 account, and stores a list of sub-accounts to store the delegated cuts of
 "real" currencies.

 For example, if I issue a currency that is 1 part dollar, 1 part gold, and 1
 part silver, then the server
 creates an issuer account in "goldbucks" and then ANY other user can create a
 "goldbucks" asset account and
 trade it like any other asset.  It doesn't even have to be a special account
 that the trader uses. It's just
 a normal account, but the instrument definition ID links to the special basket
 issuer
 account maintained by the server.

 Meanwhile, behind the scenes, the server's "goldbucks" issuer account is not
 OTAccount, but derived from it.
 suppose derived from OTAccount, that contains a list of 3 sub-accounts, 1
 denominated in the dollar asset
 specified in the contract, 1 denominiated in the gold asset, and so on.

 The OTAssetBasket contract (with sub-issuers) and the BasketAccount (issuer
 account) objects handle all the
 details of converting between the sub-accounts and the main account.

 If I am trading in goldbucks, and I have 9 goldbucks in my goldbucks account,
 then the goldbucks issuer account
 (controlled by the transaction server) must have at least -9 on its balance due
 to me. And its hidden 3 sub-accounts
 have at least +3 dollars, +3 gold, and +3 silver stored along with the rest
 that make up their total balances from
 all the users of that basket currency.

 */

namespace opentxs
{

class ServerContext;

class Basket : public Contract
{
protected:
    int32_t m_nSubCount{0};
    int64_t m_lMinimumTransfer{0};  // used in the actual basket
    int32_t m_nTransferMultiple{0}; // used in a request basket. If non-zero, that
                                 // means this is a request basket.
    Identifier m_RequestAccountID; // used in a request basket so the server
                                   // knows your acct ID.
    dequeOfBasketItems m_dequeItems;
    bool m_bHideAccountID{false}; // When saving, we might wish to produce a version
                           // without Account IDs
    // So that the resulting hash will be a consistent ID across different
    // servers.
    bool m_bExchangingIn{false}; // True if exchanging INTO the basket, False if
                          // exchanging OUT of the basket.
    int64_t m_lClosingTransactionNo{0}; // For the main (basket) account, in a
                                     // request basket (for exchanges.)
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

public:
    EXPORT Basket();
    EXPORT Basket(int32_t nCount, int64_t lMinimumTransferAmount);
    EXPORT virtual ~Basket();

    void UpdateContents() override;

    EXPORT void CalculateContractID(Identifier& newID) const override;

    inline int64_t GetMinimumTransfer() const
    {
        return m_lMinimumTransfer;
    }

    inline int32_t GetTransferMultiple() const
    {
        return m_nTransferMultiple;
    }
    inline void SetTransferMultiple(int32_t nTransferMultiple)
    {
        m_nTransferMultiple = nTransferMultiple;
    }

    inline bool IsExchanging() const
    {
        return (m_nTransferMultiple > 0);
    }

    inline bool GetExchangingIn() const
    {
        return m_bExchangingIn;
    }
    inline void SetExchangingIn(bool bDirection)
    {
        m_bExchangingIn = bDirection;
    }

    EXPORT int32_t Count() const;
    EXPORT BasketItem* At(uint32_t nIndex);

    int64_t GetClosingTransactionNoAt(uint32_t nIndex);

    inline int64_t GetClosingNum() const
    {
        return m_lClosingTransactionNo;
    }
    inline void SetClosingNum(const int64_t& lClosingNum)
    {
        m_lClosingTransactionNo = lClosingNum;
    }

    // For generating a real basket.  The user does this part, and the server
    // creates Account ID later
    // (That's why you don't see the account ID being passed in to the method.)
    EXPORT void AddSubContract(const Identifier& SUB_CONTRACT_ID,
                               int64_t lMinimumTransferAmount);
    inline void IncrementSubCount()
    {
        m_nSubCount++;
    }

    // For generating a user request to exchange in/out of a basket.
    // Assumes that SetTransferMultiple has already been called.
    EXPORT void AddRequestSubContract(const Identifier& SUB_CONTRACT_ID,
                                      const Identifier& SUB_ACCOUNT_ID,
                                      const int64_t& lClosingTransactionNo);

    inline void SetRequestAccountID(const Identifier& theAccountID)
    {
        m_RequestAccountID = theAccountID;
    }
    inline const Identifier& GetRequestAccountID()
    {
        return m_RequestAccountID;
    }

    void Release() override;
    void Release_Basket();

    // The basket itself only stores the CLOSING numbers.
    // For the opening number, you have to go deal with the exchangeBasket
    // TRANSACTION.

    // Normally do this if your transaction failed so you can get most of your
    // numbers back
    EXPORT void HarvestClosingNumbers(
        ServerContext& context,
        Nym& theNym,
        const Identifier& theNotaryID,
        bool bSave = true);

private:
    void GenerateContents(OTStringXML& xmlUnsigned, bool bHideAccountID) const;
};

} // namespace opentxs

#endif // OPENTXS_BASKET_BASKET_HPP
