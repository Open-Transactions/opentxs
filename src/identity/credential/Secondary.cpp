// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::CredentialRole

#include "identity/credential/Secondary.hpp"  // IWYU pragma: associated

#include <Credential.pb.h>
#include <Enums.pb.h>
#include <Signature.pb.h>
#include <memory>
#include <stdexcept>

#include "2_Factory.hpp"
#include "identity/credential/Key.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::SecondaryCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const crypto::Parameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
    -> identity::credential::internal::Secondary*
{
    using ReturnType = identity::credential::implementation::Secondary;

    try {

        return new ReturnType(
            api, parent, source, master, parameters, version, reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to create credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::SecondaryCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const proto::Credential& serialized)
    -> identity::credential::internal::Secondary*
{
    using ReturnType = identity::credential::implementation::Secondary;

    try {

        return new ReturnType(api, parent, source, master, serialized);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to deserialize credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::credential::implementation
{
Secondary::Secondary(
    const api::Session& api,
    const identity::internal::Authority& owner,
    const identity::Source& source,
    const internal::Primary& master,
    const crypto::Parameters& nymParameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : credential::implementation::Key(
          api,
          owner,
          source,
          nymParameters,
          version,
          identity::CredentialRole::ChildKey,
          reason,
          get_master_id(api, master))
{
    {
        Lock lock(lock_);
        first_time_init(lock);
    }

    init(master, reason);
}

Secondary::Secondary(
    const api::Session& api,
    const identity::internal::Authority& owner,
    const identity::Source& source,
    const internal::Primary& master,
    const proto::Credential& serialized) noexcept(false)
    : credential::implementation::Key(
          api,
          owner,
          source,
          serialized,
          get_master_id(api, serialized, master))
{
    Lock lock(lock_);
    init_serialized(lock);
}

auto Secondary::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
    -> std::shared_ptr<Base::SerializedType>
{
    auto serializedCredential = Key::serialize(lock, asPrivate, asSigned);

    if (asSigned) {
        auto masterSignature = MasterSignature();

        if (masterSignature) {
            *serializedCredential->add_signature() = *masterSignature;
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to get master signature.")
                .Flush();
        }
    }

    serializedCredential->set_role(proto::CREDROLE_CHILDKEY);

    return serializedCredential;
}
}  // namespace opentxs::identity::credential::implementation
