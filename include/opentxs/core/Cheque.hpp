// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CHEQUE_HPP
#define OPENTXS_CORE_CHEQUE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/OTTrackable.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace implementation
{

class Factory;

}  // namespace implementation
}  // namespace api

class Cheque : public OTTrackable
{
public:
    inline void SetAsVoucher(
        const identifier::Nym& remitterNymID,
        const Identifier& remitterAcctID)
    {
        m_REMITTER_NYM_ID = remitterNymID;
        m_REMITTER_ACCT_ID = remitterAcctID;
        m_bHasRemitter = true;
        m_strContractType = String::Factory("VOUCHER");
    }
    inline const String& GetMemo() const { return m_strMemo; }
    inline const std::int64_t& GetAmount() const { return m_lAmount; }
    inline const identifier::Nym& GetRecipientNymID() const
    {
        return m_RECIPIENT_NYM_ID;
    }
    inline bool HasRecipient() const { return m_bHasRecipient; }
    inline const identifier::Nym& GetRemitterNymID() const
    {
        return m_REMITTER_NYM_ID;
    }
    inline const Identifier& GetRemitterAcctID() const
    {
        return m_REMITTER_ACCT_ID;
    }
    inline bool HasRemitter() const { return m_bHasRemitter; }
    inline const Identifier& SourceAccountID() const
    {
        return ((m_bHasRemitter) ? m_REMITTER_ACCT_ID : m_SENDER_ACCT_ID);
    }

    // A cheque HAS NO "Recipient Asset Acct ID", since the recipient's account
    // (where he deposits
    // the cheque) is not known UNTIL the time of the deposit. It's certain not
    // known at the time
    // that the cheque is written...

    // Calling this function is like writing a check...
    EXPORT bool IssueCheque(
        const std::int64_t& lAmount,
        const std::int64_t& lTransactionNum,
        const time64_t& VALID_FROM,
        const time64_t& VALID_TO,  // The expiration date (valid from/to dates.)
        const Identifier& SENDER_ACCT_ID,  // The asset account the cheque is
                                           // drawn on.
        const identifier::Nym& SENDER_NYM_ID,  // This ID must match the user ID
                                               // on the asset account,
        // AND must verify the cheque signature with that user's key.
        const String& strMemo,                      // Optional memo field.
        const identifier::Nym& pRECIPIENT_NYM_ID);  // Recipient
                                                    // optional. (Might
                                                    // be a blank
                                                    // cheque.)

    EXPORT void CancelCheque();  // You still need to re-sign the cheque after
                                 // doing this.

    void InitCheque();
    void Release() override;
    void Release_Cheque();
    void UpdateContents() override;  // Before transmission or serialization,
                                     // this is where the token saves its
                                     // contents

    EXPORT virtual ~Cheque();

protected:
    Amount m_lAmount{0};
    OTString m_strMemo;
    // Optional. If present, must match depositor's user ID.
    OTNymID m_RECIPIENT_NYM_ID;
    bool m_bHasRecipient{false};
    // In the case of vouchers (cashier's cheques) we store the Remitter's ID.
    OTNymID m_REMITTER_NYM_ID;
    OTIdentifier m_REMITTER_ACCT_ID;
    bool m_bHasRemitter{false};

    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

private:  // Private prevents erroneous use by other classes.
    friend api::implementation::Factory;

    typedef OTTrackable ot_super;

    EXPORT Cheque(const api::Core& core);
    EXPORT Cheque(
        const api::Core& core,
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID);

    Cheque() = delete;
};
}  // namespace opentxs
#endif
