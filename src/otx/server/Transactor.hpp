// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/otx/AccountList.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class UnitDefinition;
}  // namespace identifier

namespace otx
{
namespace context
{
class Client;
}  // namespace context
}  // namespace otx

namespace server
{
class MainFile;
class Server;
}  // namespace server

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::server
{
class Transactor
{
public:
    auto issueNextTransactionNumber(TransactionNumber& txNumber) -> bool;
    auto issueNextTransactionNumberToNym(
        otx::context::Client& context,
        TransactionNumber& txNumber) -> bool;

    auto transactionNumber() const -> TransactionNumber
    {
        return transaction_number_;
    }

    void transactionNumber(TransactionNumber value)
    {
        transaction_number_ = value;
    }

    auto addBasketAccountID(
        const identifier::Generic& basketId,
        const identifier::Generic& basketAccountId,
        const identifier::Generic& basketContractId) -> bool;
    auto lookupBasketAccountID(
        const identifier::Generic& basketId,
        identifier::Generic& basketAccountId) -> bool;

    auto lookupBasketAccountIDByContractID(
        const identifier::Generic& basketContractId,
        identifier::Generic& basketAccountId) -> bool;
    auto lookupBasketContractIDByAccountID(
        const identifier::Generic& basketAccountId,
        identifier::Generic& basketContractId) -> bool;

    // Whenever the server issues a voucher (like a cashier's cheque), it puts
    // the funds in one of these voucher accounts (one for each instrument
    // definition ID). Then it issues the cheque from the same account.
    // TODO: also should save the cheque itself to a folder, where the folder is
    // named based on the date that the cheque will expire.  This way, the
    // server operator can go back later, or have a script, to retrieve the
    // cheques from the expired folders, and total them. The server operator is
    // free to remove that total from the Voucher Account once the cheque has
    // expired: it is his money now.
    auto getVoucherAccount(
        const identifier::UnitDefinition& instrumentDefinitionID)
        -> ExclusiveAccount;

    Transactor(Server& server, const PasswordPrompt& reason);
    Transactor() = delete;

    ~Transactor() = default;

private:
    friend MainFile;

    using BasketsMap = UnallocatedMap<UnallocatedCString, UnallocatedCString>;

    Server& server_;
    const PasswordPrompt& reason_;
    // This stores the last VALID AND ISSUED transaction number.
    TransactionNumber transaction_number_;
    // maps basketId with basketAccountId
    BasketsMap id_to_basket_map_;
    // basket issuer account ID, which is *different* on each server, using the
    // Basket Currency's ID, which is the *same* on every server.)
    // Need a way to look up a Basket Account ID using its Contract ID
    BasketsMap contract_id_to_basket_account_id_;
    // The list of voucher accounts (see GetVoucherAccount below for details)
    otx::internal::AccountList voucher_accounts_;
};
}  // namespace opentxs::server
