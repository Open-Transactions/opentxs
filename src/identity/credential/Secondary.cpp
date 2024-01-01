// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/credential/Secondary.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Credential.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <memory>
#include <stdexcept>

#include "identity/credential/Key.hpp"
#include "opentxs/api/session/Crypto.hpp"       // IWYU pragma: keep
#include "opentxs/identity/CredentialRole.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
#include "opentxs/internal.factory.hpp"
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
        LogError()()("Failed to create credential: ")(e.what()).Flush();

        return nullptr;
    }
}

auto Factory::SecondaryCredential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const protobuf::Credential& serialized)
    -> identity::credential::internal::Secondary*
{
    using ReturnType = identity::credential::implementation::Secondary;

    try {

        return new ReturnType(api, parent, source, master, serialized);
    } catch (const std::exception& e) {
        LogError()()("Failed to deserialize credential: ")(e.what()).Flush();

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
          master.ID())
{
    first_time_init(set_name_from_id_);
    init(master, reason);
}

Secondary::Secondary(
    const api::Session& api,
    const identity::internal::Authority& owner,
    const identity::Source& source,
    const internal::Primary& master,
    const protobuf::Credential& serialized) noexcept(false)
    : credential::implementation::Key(
          api,
          owner,
          source,
          serialized,
          get_master_id(api, serialized, master))
{
    init_serialized();
}

auto Secondary::id_form() const -> std::shared_ptr<SerializedType>
{
    auto out = Key::id_form();
    out->set_role(protobuf::CREDROLE_CHILDKEY);

    return out;
}

auto Secondary::serialize(
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
    -> std::shared_ptr<Base::SerializedType>
{
    auto out = Key::serialize(asPrivate, asSigned);

    if (asSigned) {
        if (auto sig = MasterSignature(); sig) {
            *out->add_signature() = *sig;
        } else {
            LogError()()("Failed to get master signature.").Flush();
        }
    }

    return out;
}
}  // namespace opentxs::identity::credential::implementation
