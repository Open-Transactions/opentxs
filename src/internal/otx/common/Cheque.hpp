// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/OTTrackable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
namespace imp
{
class Factory;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

class PasswordPrompt;

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
    inline auto GetMemo() const -> const String& { return m_strMemo; }
    inline auto GetAmount() const -> const Amount& { return m_lAmount; }
    inline auto GetRecipientNymID() const -> const identifier::Nym&
    {
        return m_RECIPIENT_NYM_ID;
    }
    inline auto HasRecipient() const -> bool { return m_bHasRecipient; }
    inline auto GetRemitterNymID() const -> const identifier::Nym&
    {
        return m_REMITTER_NYM_ID;
    }
    inline auto GetRemitterAcctID() const -> const Identifier&
    {
        return m_REMITTER_ACCT_ID;
    }
    inline auto HasRemitter() const -> bool { return m_bHasRemitter; }
    inline auto SourceAccountID() const -> const Identifier&
    {
        return ((m_bHasRemitter) ? m_REMITTER_ACCT_ID : m_SENDER_ACCT_ID);
    }

    // A cheque HAS NO "Recipient Asset Acct ID", since the recipient's account
    // (where he deposits
    // the cheque) is not known UNTIL the time of the deposit. It's certain not
    // known at the time
    // that the cheque is written...

    // Calling this function is like writing a check...
    auto IssueCheque(
        const Amount& lAmount,
        const std::int64_t& lTransactionNum,
        const Time& VALID_FROM,
        const Time& VALID_TO,  // The expiration date (valid from/to dates.)
        const Identifier& SENDER_ACCT_ID,  // The asset account the cheque is
                                           // drawn on.
        const identifier::Nym& SENDER_NYM_ID,  // This ID must match the user ID
                                               // on the asset account,
        // AND must verify the cheque signature with that user's key.
        const String& strMemo,  // Optional memo field.
        const identifier::Nym& pRECIPIENT_NYM_ID) -> bool;  // Recipient
                                                            // optional. (Might
                                                            // be a blank
                                                            // cheque.)

    void CancelCheque();  // You still need to re-sign the cheque
                          // after doing this.

    void InitCheque();
    void Release() override;
    void Release_Cheque();
    void UpdateContents(
        const PasswordPrompt& reason) override;  // Before transmission or
                                                 // serialization, this is where
                                                 // the token saves its contents

    ~Cheque() override;

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

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

private:  // Private prevents erroneous use by other classes.
    friend api::session::imp::Factory;

    using ot_super = OTTrackable;

    Cheque(const api::Session& api);
    Cheque(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);

    Cheque() = delete;
};
}  // namespace opentxs
