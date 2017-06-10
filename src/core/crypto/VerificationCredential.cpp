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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/VerificationCredential.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <memory>
#include <ostream>
#include <string>

namespace opentxs
{

// static
proto::Verification VerificationCredential::SigningForm(
    const proto::Verification& item)
{
    proto::Verification signingForm(item);
    signingForm.clear_sig();

    return signingForm;
}

// static
std::string VerificationCredential::VerificationID(
    const proto::Verification& item)
{
    Identifier id;
    id.CalculateDigest(proto::ProtoAsData<proto::Verification>(item));

    return String(id).Get();
}

VerificationCredential::VerificationCredential(
    CredentialSet& parent,
    const proto::Credential& credential)
    : ot_super(parent, credential)
{
    mode_ = proto::KEYMODE_NULL;
    master_id_ = String(credential.childdata().masterid());
    data_.reset(new proto::VerificationSet(credential.verification()));
}

VerificationCredential::VerificationCredential(
    CredentialSet& parent,
    const NymParameters& nymParameters)
    : ot_super(parent, 1, nymParameters)
{
    mode_ = proto::KEYMODE_NULL;
    role_ = proto::CREDROLE_VERIFY;
    nym_id_ = parent.GetNymID();
    master_id_ = parent.GetMasterCredID();
    auto verificationSet = nymParameters.VerificationSet();

    if (verificationSet) {
        data_.reset(new proto::VerificationSet(*verificationSet));
    }
}

bool VerificationCredential::GetVerificationSet(
    std::unique_ptr<proto::VerificationSet>& verificationSet) const
{
    if (!data_) {
        return false;
    }

    verificationSet.reset(new proto::VerificationSet(*data_));

    return true;
}

serializedCredential VerificationCredential::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::serialize(lock, asPrivate, asSigned);
    serializedCredential->set_mode(proto::KEYMODE_NULL);
    serializedCredential->clear_signature();  // this fixes a bug, but shouldn't

    if (asSigned) {
        SerializedSignature masterSignature = MasterSignature();

        if (masterSignature) {
            // We do not own this pointer.
            proto::Signature* serializedMasterSignature =
                serializedCredential->add_signature();
            *serializedMasterSignature = *masterSignature;
        } else {
            otErr << __FUNCTION__ << ": Failed to get master signature.\n";
        }
    }

    *(serializedCredential->mutable_verification()) = *data_;

    return serializedCredential;
}

bool VerificationCredential::verify_internally(const Lock& lock) const
{
    // Perform common Credential verifications
    if (!ot_super::verify_internally(lock)) { return false; }

    if (data_) {
        for (auto& nym : data_->internal().identity()) {
            for (auto& claim : nym.verification()) {
                bool valid = owner_backlink_->Verify(claim);

                if (!valid) {
                    otErr << __FUNCTION__ << ": invalid claim verification."
                          << std::endl;

                    return false;
                }
            }
        }
    }

    return true;
}

}  // namespace opentxs
