// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/basket/BasketItem.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class FactoryPrivate;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class PasswordPrompt;
class StringXML;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
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
class Basket final : public Contract
{
public:
    void UpdateContents(const PasswordPrompt& reason) final;

    void CalculateContractID(identifier::Generic& newID) const final;

    inline auto GetMinimumTransfer() const -> const Amount&
    {
        return minimum_transfer_;
    }

    inline auto GetTransferMultiple() const -> std::int32_t
    {
        return transfer_multiple_;
    }
    inline void SetTransferMultiple(std::int32_t nTransferMultiple)
    {
        transfer_multiple_ = nTransferMultiple;
    }

    inline auto IsExchanging() const -> bool
    {
        return (transfer_multiple_ > 0);
    }

    inline auto GetExchangingIn() const -> bool { return exchanging_in_; }
    inline void SetExchangingIn(bool bDirection)
    {
        exchanging_in_ = bDirection;
    }

    auto Count() const -> std::int32_t;
    auto At(std::uint32_t nIndex) -> BasketItem*;

    auto GetClosingTransactionNoAt(std::uint32_t nIndex) -> std::int64_t;

    inline auto GetClosingNum() const -> std::int64_t
    {
        return closing_transaction_no_;
    }
    inline void SetClosingNum(const std::int64_t& lClosingNum)
    {
        closing_transaction_no_ = lClosingNum;
    }

    // For generating a real basket.  The user does this part, and the server
    // creates Account ID later
    // (That's why you don't see the account ID being passed in to the method.)
    void AddSubContract(
        const identifier::Generic& SUB_CONTRACT_ID,
        std::int64_t lMinimumTransferAmount);
    inline void IncrementSubCount() { sub_count_++; }

    // For generating a user request to exchange in/out of a basket.
    // Assumes that SetTransferMultiple has already been called.
    void AddRequestSubContract(
        const identifier::Generic& SUB_CONTRACT_ID,
        const identifier::Account& SUB_ACCOUNT_ID,
        const std::int64_t& lClosingTransactionNo);

    inline void SetRequestAccountID(const identifier::Account& theAccountID)
    {
        request_account_id_ = theAccountID;
    }
    inline auto GetRequestAccountID() -> const identifier::Account&
    {
        return request_account_id_;
    }

    void Release() final;
    void Release_Basket();

    // The basket itself only stores the CLOSING numbers.
    // For the opening number, you have to go deal with the exchangeBasket
    // TRANSACTION.

    // Normally do this if your transaction failed so you can get most of your
    // numbers back
    void HarvestClosingNumbers(
        otx::context::Server& context,
        const identifier::Notary& theNotaryID,
        bool bSave = true);

    Basket() = delete;

    ~Basket() final;

protected:
    std::int32_t sub_count_{0};
    // used in the actual basket
    Amount minimum_transfer_{0};
    // used in a request basket. If non-zero, that means this is a request
    // basket.
    std::int32_t transfer_multiple_{0};
    // used in a request basket so the server knows your acct ID.
    identifier::Account request_account_id_;
    dequeOfBasketItems items_;
    // When saving, we might wish to produce a version without Account IDs
    // So that the resulting hash will be a consistent ID across different
    // servers.
    bool hide_account_id_{false};
    // True if exchanging INTO the basket, False if exchanging OUT of the
    // basket.
    bool exchanging_in_{false};
    // For the main (basket) account, in a request basket (for exchanges.)
    TransactionNumber closing_transaction_no_{0};
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t final;

private:
    friend api::session::FactoryPrivate;

    Basket(const api::Session& api);
    Basket(
        const api::Session& api,
        std::int32_t nCount,
        const Amount& lMinimumTransferAmount);

    void GenerateContents(StringXML& xmlUnsigned, bool bHideAccountID) const;
};
}  // namespace opentxs
