// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/OTTrackable.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Instrument.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>

namespace opentxs
{
OTTrackable::OTTrackable(const api::Core& core)
    : Instrument(core)
    , m_lTransactionNum(0)
    , m_SENDER_ACCT_ID(Identifier::Factory())
    , m_SENDER_NYM_ID(identifier::Nym::Factory())
{
    InitTrackable();
}

OTTrackable::OTTrackable(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID)
    : Instrument(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lTransactionNum(0)
    , m_SENDER_ACCT_ID(Identifier::Factory())
    , m_SENDER_NYM_ID(identifier::Nym::Factory())
{
    InitTrackable();
}

OTTrackable::OTTrackable(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& ACCT_ID,
    const identifier::Nym& NYM_ID)
    : Instrument(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lTransactionNum(0)
    , m_SENDER_ACCT_ID(Identifier::Factory())
    , m_SENDER_NYM_ID(identifier::Nym::Factory())
{
    InitTrackable();

    SetSenderAcctID(ACCT_ID);
    SetSenderNymID(NYM_ID);
}

void OTTrackable::InitTrackable()
{
    // Should never happen in practice. A child class will override it.
    m_strContractType->Set("TRACKABLE");
    m_lTransactionNum = 0;
}

bool OTTrackable::HasTransactionNum(const std::int64_t& lInput) const
{
    return lInput == m_lTransactionNum;
}

void OTTrackable::GetAllTransactionNumbers(NumList& numlistOutput) const
{
    if (m_lTransactionNum > 0) numlistOutput.Add(m_lTransactionNum);
}

void OTTrackable::Release_Trackable()
{
    m_SENDER_ACCT_ID->Release();
    m_SENDER_NYM_ID->Release();
}

void OTTrackable::Release()
{
    Release_Trackable();
    Instrument::Release();

    // Then I call this to re-initialize everything for myself.
    InitTrackable();
}

void OTTrackable::SetSenderAcctID(const Identifier& ACCT_ID)
{
    m_SENDER_ACCT_ID = Identifier::Factory(ACCT_ID);
}

void OTTrackable::SetSenderNymID(const identifier::Nym& NYM_ID)
{
    m_SENDER_NYM_ID = (NYM_ID);
}

void OTTrackable::UpdateContents() {}

OTTrackable::~OTTrackable() { Release_Trackable(); }
}  // namespace opentxs
