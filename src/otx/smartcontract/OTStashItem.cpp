// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/smartcontract/OTStashItem.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/core/String.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
OTStashItem::OTStashItem(const api::Session& api)
    : api_(api)
    , instrument_definition_id_(String::Factory())
    , amount_(0)

{
}

OTStashItem::OTStashItem(
    const api::Session& api,
    const String& strInstrumentDefinitionID,
    std::int64_t lAmount)
    : api_(api)
    , instrument_definition_id_(strInstrumentDefinitionID)
    , amount_(lAmount)
{
}

OTStashItem::OTStashItem(
    const api::Session& api,
    const identifier::Generic& theInstrumentDefinitionID,
    std::int64_t lAmount)
    : api_(api)
    , instrument_definition_id_(
          String::Factory(theInstrumentDefinitionID, api_.Crypto()))
    , amount_(lAmount)
{
}

OTStashItem::~OTStashItem() = default;

/*
 IDEA: todo security.

 Make a base class that keeps the amount itself PRIVATE, so even its subclasses
 can't see it.

 This is where Credit() and Debit() are made available as PROTECTED, so that its
 subclasses can USE them
 to manipulate the amount, which they can't otherwise see directly at all.

 This thing should be able to SERIALIZE itself as part of a bigger class.

 Actually Credit and Debit should be PUBLIC so that people can use instances of
 this class
 without having to subclass from it.

 Then I can use it ALL OVER THE PLACE where Balances are:  Accounts, Stashes,
 Instruments, etc.

 */

auto OTStashItem::CreditStash(const std::int64_t& lAmount) -> bool
{
    if (lAmount < 0) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed attempt to credit a "
                "negative amount (")(lAmount)("). Asset Type: ")(
                instrument_definition_id_.get())(".")
                .Flush();
        }
        return false;
    }

    amount_ += lAmount;

    return true;
}

auto OTStashItem::DebitStash(const std::int64_t& lAmount) -> bool
{
    if (lAmount < 0) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed attempt to debit a "
                "negative amount (")(lAmount)("). Asset Type: ")(
                instrument_definition_id_.get())(".")
                .Flush();
        }
        return false;
    }

    const std::int64_t lTentativeNewBalance = (amount_ - lAmount);

    if (lTentativeNewBalance < 0) {
        {
            LogConsole()(OT_PRETTY_CLASS())(
                "Failed attempt to debit (amount of) ")(
                lAmount)(": New stash balance would have been a negative "
                         "amount (")(lTentativeNewBalance)("). Asset Type: ")(
                instrument_definition_id_.get())(".")
                .Flush();
        }
        return false;
    }

    amount_ = lTentativeNewBalance;

    return true;
}

}  // namespace opentxs
