// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/VerificationCredential.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::VerificationCredential::"

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
    auto id = Identifier::Factory();
    id->CalculateDigest(proto::ProtoAsData<proto::Verification>(item));

    return String::Factory(id)->Get();
}

VerificationCredential::VerificationCredential(
    const api::Core& api,
    CredentialSet& parent,
    const proto::Credential& credential)
    : ot_super(api, parent, credential)
{
    mode_ = proto::KEYMODE_NULL;
    master_id_ = credential.childdata().masterid();
    data_.reset(new proto::VerificationSet(credential.verification()));
}

VerificationCredential::VerificationCredential(
    const api::Core& api,
    CredentialSet& parent,
    const NymParameters& nymParameters)
    : ot_super(api, parent, VERIFICATION_CREDENTIAL_VERSION, nymParameters)
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
    if (!data_) { return false; }

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
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to get master signature.")
                .Flush();
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
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid claim verification.")
                        .Flush();

                    return false;
                }
            }
        }
    }

    return true;
}

}  // namespace opentxs
