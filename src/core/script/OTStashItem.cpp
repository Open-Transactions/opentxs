// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/script/OTStashItem.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>
#include <ostream>

#define OT_METHOD "opentxs::OTStashItem"
namespace opentxs
{

/*
 std::int64_t GetAmount() const { return m_lAmount; }
 void SetAmount( std::int64_t lAmount) { m_lAmount = lAmount; }

 const OTString& GetInstrumentDefinitionID() { return
 m_strInstrumentDefinitionID; }
 */

OTStashItem::OTStashItem()
    : m_strInstrumentDefinitionID(String::Factory())
    , m_lAmount(0)

{
}

OTStashItem::OTStashItem(
    const String& strInstrumentDefinitionID,
    std::int64_t lAmount)
    : m_strInstrumentDefinitionID(strInstrumentDefinitionID)
    , m_lAmount(lAmount)
{
}

OTStashItem::OTStashItem(
    const Identifier& theInstrumentDefinitionID,
    std::int64_t lAmount)
    : m_strInstrumentDefinitionID(String::Factory(theInstrumentDefinitionID))
    , m_lAmount(lAmount)
{
}

OTStashItem::~OTStashItem() {}

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

bool OTStashItem::CreditStash(const std::int64_t& lAmount)
{
    if (lAmount < 0) {
        {
	    LogNormal(OT_METHOD)(__FUNCTION__)(
	      ": Failed attempt to credit a "
	      "negative amount (")
              (lAmount)("). Asset Type: ")(m_strInstrumentDefinitionID).Flush();
          }
        return false;
    }

    m_lAmount += lAmount;

    return true;
}

bool OTStashItem::DebitStash(const std::int64_t& lAmount)
{
    if (lAmount < 0) {
        {
	    LogNormal(OT_METHOD)(__FUNCTION__)(
	      ": Failed attempt to debit a "
	      "negative amount (")
              (lAmount)("). Asset Type: ")(m_strInstrumentDefinitionID).Flush();
          }
        return false;
    }

    const std::int64_t lTentativeNewBalance = (m_lAmount - lAmount);

    if (lTentativeNewBalance < 0) {
        {
	    LogNormal(OT_METHOD)(__FUNCTION__)(
	      ": Failed attempt to debit (amount of) ")
              (lAmount)
              (": New stash balance would have been a negative "
	      "amount (")
              (lTentativeNewBalance)
              ("). Asset Type: ")(m_strInstrumentDefinitionID).Flush();
          }
        return false;
    }

    m_lAmount = lTentativeNewBalance;

    return true; 
}

}  // namespace opentxs
