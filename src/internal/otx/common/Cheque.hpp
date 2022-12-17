// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "internal/core/String.hpp"
#include "internal/otx/common/OTTrackable.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class Cheque : public OTTrackable
{
public:
    inline void SetAsVoucher(
        const identifier::Nym& remitterNymID,
        const identifier::Generic& remitterAcctID)
    {
        remitter_nym_id_ = remitterNymID;
        remitter_account_id_ = remitterAcctID;
        has_remitter_ = true;
        contract_type_ = String::Factory("VOUCHER");
    }
    inline auto GetMemo() const -> const String& { return memo_; }
    inline auto GetAmount() const -> const Amount& { return amount_; }
    inline auto GetRecipientNymID() const -> const identifier::Nym&
    {
        return recipient_nym_id_;
    }
    inline auto HasRecipient() const -> bool { return has_recipient_; }
    inline auto GetRemitterNymID() const -> const identifier::Nym&
    {
        return remitter_nym_id_;
    }
    inline auto GetRemitterAcctID() const -> const identifier::Generic&
    {
        return remitter_account_id_;
    }
    inline auto HasRemitter() const -> bool { return has_remitter_; }
    inline auto SourceAccountID() const -> const identifier::Generic&
    {
        return ((has_remitter_) ? remitter_account_id_ : sender_account_id_);
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
        const identifier::Generic& SENDER_ACCT_ID,  // The asset account the
                                                    // cheque is drawn on.
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

    Cheque() = delete;

    ~Cheque() override;

protected:
    Amount amount_{0};
    OTString memo_;
    // Optional. If present, must match depositor's user ID.
    identifier::Nym recipient_nym_id_;
    bool has_recipient_{false};
    // In the case of vouchers (cashier's cheques) we store the Remitter's ID.
    identifier::Nym remitter_nym_id_;
    identifier::Generic remitter_account_id_;
    bool has_remitter_{false};

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

private:  // Private prevents erroneous use by other classes.
    friend api::session::imp::Factory;

    using ot_super = OTTrackable;

    Cheque(const api::Session& api);
    Cheque(
        const api::Session& api,
        const identifier::Notary& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
};
}  // namespace opentxs
