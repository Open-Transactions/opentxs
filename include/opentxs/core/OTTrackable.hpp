// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_OTTRACKABLE_HPP
#define OPENTXS_CORE_OTTRACKABLE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Instrument.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
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
    void UpdateContents() override;

    virtual bool HasTransactionNum(const TransactionNumber& lInput) const;
    virtual void GetAllTransactionNumbers(NumList& numlistOutput) const;

    inline TransactionNumber GetTransactionNum() const
    {
        return m_lTransactionNum;
    }

    inline void SetTransactionNum(TransactionNumber lTransactionNum)
    {
        m_lTransactionNum = lTransactionNum;
    }

    inline const Identifier& GetSenderAcctID() const
    {
        return m_SENDER_ACCT_ID;
    }

    inline const identifier::Nym& GetSenderNymID() const
    {
        return m_SENDER_NYM_ID;
    }

    virtual ~OTTrackable();

protected:
    TransactionNumber m_lTransactionNum{0};
    // The asset account the instrument is drawn on.
    OTIdentifier m_SENDER_ACCT_ID;
    // This ID must match the user ID on that asset account,
    // AND must verify the instrument's signature with that user's key.
    OTNymID m_SENDER_NYM_ID;

    void SetSenderAcctID(const Identifier& ACCT_ID);
    void SetSenderNymID(const identifier::Nym& NYM_ID);

    OTTrackable(const api::Core& core);
    OTTrackable(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID);
    OTTrackable(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& ACCT_ID,
        const identifier::Nym& NYM_ID);

private:
    OTTrackable() = delete;
};
}  // namespace opentxs
#endif
