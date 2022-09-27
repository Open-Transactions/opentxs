// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "internal/otx/common/OTTrackable.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/otx/common/Instrument.hpp"
#include "internal/otx/common/NumList.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs
{
OTTrackable::OTTrackable(const api::Session& api)
    : Instrument(api)
    , transaction_num_(0)
    , sender_account_id_()
    , sender_nym_id_()
{
    InitTrackable();
}

OTTrackable::OTTrackable(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    : Instrument(api, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , transaction_num_(0)
    , sender_account_id_()
    , sender_nym_id_()
{
    InitTrackable();
}

OTTrackable::OTTrackable(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::Generic& ACCT_ID,
    const identifier::Nym& NYM_ID)
    : Instrument(api, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , transaction_num_(0)
    , sender_account_id_()
    , sender_nym_id_()
{
    InitTrackable();

    SetSenderAcctID(ACCT_ID);
    SetSenderNymID(NYM_ID);
}

void OTTrackable::InitTrackable()
{
    // Should never happen in practice. A child class will override it.
    contract_type_->Set("TRACKABLE");
    transaction_num_ = 0;
}

auto OTTrackable::HasTransactionNum(const std::int64_t& lInput) const -> bool
{
    return lInput == transaction_num_;
}

void OTTrackable::GetAllTransactionNumbers(NumList& numlistOutput) const
{
    if (transaction_num_ > 0) { numlistOutput.Add(transaction_num_); }
}

void OTTrackable::Release_Trackable()
{
    sender_account_id_.clear();
    sender_nym_id_.clear();
}

void OTTrackable::Release()
{
    Release_Trackable();
    Instrument::Release();

    // Then I call this to re-initialize everything for myself.
    InitTrackable();
}

void OTTrackable::SetSenderAcctID(const identifier::Generic& ACCT_ID)
{
    sender_account_id_ = ACCT_ID;
}

void OTTrackable::SetSenderNymID(const identifier::Nym& NYM_ID)
{
    sender_nym_id_ = NYM_ID;
}

void OTTrackable::UpdateContents(const PasswordPrompt& reason) {}

OTTrackable::~OTTrackable() { Release_Trackable(); }
}  // namespace opentxs
