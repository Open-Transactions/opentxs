// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/otx/common/Instrument.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

class NumList;
class PasswordPrompt;

// OTTrackable is very similar to OTInstrument.
// The difference is, it may have identifying info on it:
// TRANSACTION NUMBER, SENDER USER ID (NYM ID), AND SENDER ACCOUNT ID.
//
class OTTrackable : public Instrument
{
public:
    void InitTrackable();
    void Release_Trackable();

    void Release() override;
    void UpdateContents(const PasswordPrompt& reason) override;

    virtual auto HasTransactionNum(const TransactionNumber& lInput) const
        -> bool;
    virtual void GetAllTransactionNumbers(NumList& numlistOutput) const;

    inline auto GetTransactionNum() const -> TransactionNumber
    {
        return m_lTransactionNum;
    }

    inline void SetTransactionNum(TransactionNumber lTransactionNum)
    {
        m_lTransactionNum = lTransactionNum;
    }

    inline auto GetSenderAcctID() const -> const Identifier&
    {
        return m_SENDER_ACCT_ID;
    }

    inline auto GetSenderNymID() const -> const identifier::Nym&
    {
        return m_SENDER_NYM_ID;
    }

    ~OTTrackable() override;

protected:
    TransactionNumber m_lTransactionNum{0};
    // The asset account the instrument is drawn on.
    OTIdentifier m_SENDER_ACCT_ID;
    // This ID must match the user ID on that asset account,
    // AND must verify the instrument's signature with that user's key.
    OTNymID m_SENDER_NYM_ID;

    void SetSenderAcctID(const Identifier& ACCT_ID);
    void SetSenderNymID(const identifier::Nym& NYM_ID);

    OTTrackable(const api::Session& api);
    OTTrackable(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
    OTTrackable(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const Identifier& ACCT_ID,
        const identifier::Nym& NYM_ID);

private:
    OTTrackable() = delete;
};
}  // namespace opentxs
