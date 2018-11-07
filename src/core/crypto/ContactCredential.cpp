// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/ContactCredential.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"

#include <ostream>
#include <string>

#define OT_METHOD "opentxs::ContactCredential::"

namespace opentxs
{
// static
std::string ContactCredential::ClaimID(
    const std::string& nymid,
    const std::uint32_t section,
    const proto::ContactItem& item)
{
    proto::Claim preimage;
    preimage.set_version(1);
    preimage.set_nymid(nymid);
    preimage.set_section(section);
    preimage.set_type(item.type());
    preimage.set_start(item.start());
    preimage.set_end(item.end());
    preimage.set_value(item.value());

    return String::Factory(ClaimID(preimage))->Get();
}

// static
std::string ContactCredential::ClaimID(
    const std::string& nymid,
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const std::int64_t start,
    const std::int64_t end,
    const std::string& value)
{
    proto::Claim preimage;
    preimage.set_version(1);
    preimage.set_nymid(nymid);
    preimage.set_section(section);
    preimage.set_type(type);
    preimage.set_start(start);
    preimage.set_end(end);
    preimage.set_value(value);

    return String::Factory(ClaimID(preimage))->Get();
}

// static
OTIdentifier ContactCredential::ClaimID(const proto::Claim& preimage)
{
    auto output = Identifier::Factory();
    output->CalculateDigest(proto::ProtoAsData<proto::Claim>(preimage));

    return output;
}

// static
Claim ContactCredential::asClaim(
    const String& nymid,
    const std::uint32_t section,
    const proto::ContactItem& item)
{
    std::set<std::uint32_t> attributes;

    for (auto& attrib : item.attribute()) { attributes.insert(attrib); }

    return Claim{ClaimID(nymid.Get(), section, item),
                 section,
                 item.type(),
                 item.value(),
                 item.start(),
                 item.end(),
                 attributes};
}

ContactCredential::ContactCredential(
    const api::Core& api,
    CredentialSet& parent,
    const proto::Credential& credential)
    : ot_super(api, parent, credential)
{
    mode_ = proto::KEYMODE_NULL;
    master_id_ = credential.childdata().masterid();
    data_.reset(new proto::ContactData(credential.contactdata()));
}

ContactCredential::ContactCredential(
    const api::Core& api,
    CredentialSet& parent,
    const NymParameters& nymParameters)
    : ot_super(api, parent, CONTACT_CREDENTIAL_VERSION, nymParameters)
{
    mode_ = proto::KEYMODE_NULL;
    role_ = proto::CREDROLE_CONTACT;
    nym_id_ = parent.GetNymID();
    master_id_ = parent.GetMasterCredID();
    auto contacts = nymParameters.ContactData();

    if (contacts) { data_.reset(new proto::ContactData(*contacts)); }
}

bool ContactCredential::GetContactData(
    std::unique_ptr<proto::ContactData>& contactData) const
{
    if (!data_) { return false; }

    contactData.reset(new proto::ContactData(*data_));

    return bool(contactData);
}

serializedCredential ContactCredential::serialize(
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

    *(serializedCredential->mutable_contactdata()) = *data_;

    return serializedCredential;
}

}  // namespace opentxs
