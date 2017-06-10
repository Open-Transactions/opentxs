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

#ifndef OPENTXS_EXT_INSTANTIATECONTRACT_HPP
#define OPENTXS_EXT_INSTANTIATECONTRACT_HPP

// Factory (though rarely used; was just added recently for the API.)
//
// If you want to instantiate a contract that you already have in string form,
// this function will figure out what kind of contract it is, and instantiate
// the right subclass, then load it up and return it.
//
// CALLER IS RESPONSIBLE to cleanup!

#include "opentxs/cash/Mint.hpp"
#include "opentxs/cash/Token.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"

#include <memory>

namespace
{

opentxs::Contract* InstantiateContract(opentxs::String strInput)
{

    using namespace opentxs;
    String strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {

        std::unique_ptr<Contract> pContract;

        if (strFirstLine.Contains(
                "-----BEGIN SIGNED SMARTCONTRACT-----")) // this string is 36
                                                         // chars long.
        {
            pContract.reset(new OTSmartContract());
            OT_ASSERT(pContract);
        }

        if (strFirstLine.Contains(
                "-----BEGIN SIGNED PAYMENT PLAN-----")) // this string is 35
                                                        // chars long.
        {
            pContract.reset(new OTPaymentPlan());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED TRADE-----")) // this string is 28
                                                        // chars long.
        {
            pContract.reset(new OTTrade());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED OFFER-----")) {
            pContract.reset(new OTOffer());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED INVOICE-----")) {
            pContract.reset(new Cheque());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract.reset(new Cheque());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract.reset(new Cheque());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract.reset(new Message());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED MINT-----")) {
            pContract.reset(Mint::MintFactory());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED FILE-----")) {
            pContract.reset(new OTSignedFile());
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----")) {
            pContract.reset(Token::LowLevelInstantiate(strFirstLine));
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED CASH TOKEN-----")) {
            pContract.reset(Token::LowLevelInstantiate(strFirstLine));
            OT_ASSERT(pContract);
        }
        else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED LUCRE CASH TOKEN-----")) {
            pContract.reset(Token::LowLevelInstantiate(strFirstLine));
            OT_ASSERT(pContract);
        }

        // The string didn't match any of the options in the factory.
        //
        if (!pContract) {
            otOut << __FUNCTION__
                  << ": Object type not yet supported by class factory: "
                  << strFirstLine << "\n";
        // Does the contract successfully load from the string passed in?
        } else if (!pContract->LoadContractFromString(strContract)) {
            otOut << __FUNCTION__
                  << ": Failed loading contract from string (first line): "
                  << strFirstLine << "\n";
        } else {
            return pContract.release();
        }
    }
    return nullptr;
}

} // unnamed namespace

#endif // OPENTXS_EXT_INSTANTIATECONTRACT_HPP
