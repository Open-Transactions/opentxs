// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/////////////////////////////////////////////////////////////////////

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 crypto::key::Asymmetrics (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "stdafx.hpp"

#include "opentxs/core/crypto/ChildKeyCredential.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <memory>
#include <ostream>

#define OT_METHOD "opentxs::ChildKeyCredential::"

namespace opentxs
{
ChildKeyCredential::ChildKeyCredential(
    const api::Core& api,
    CredentialSet& owner,
    const proto::Credential& serializedCred)
    : ot_super(api, owner, serializedCred)
{
    role_ = proto::CREDROLE_CHILDKEY;
    master_id_ = serializedCred.childdata().masterid();
}

ChildKeyCredential::ChildKeyCredential(
    const api::Core& api,
    CredentialSet& owner,
    const NymParameters& nymParameters)
    : ot_super(api, owner, nymParameters)
{
    role_ = proto::CREDROLE_CHILDKEY;

    nym_id_ = owner.GetNymID();
    master_id_ = owner.GetMasterCredID();
}

serializedCredential ChildKeyCredential::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = ot_super::serialize(lock, asPrivate, asSigned);

    if (asSigned) {
        auto masterSignature = MasterSignature();

        if (masterSignature) {
            *serializedCredential->add_signature() = *masterSignature;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to get master signature.")
                .Flush();
        }
    }

    serializedCredential->set_role(proto::CREDROLE_CHILDKEY);

    return serializedCredential;
}
}  // namespace opentxs
