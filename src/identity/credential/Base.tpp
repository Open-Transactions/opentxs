// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/credential/Base.hpp"  // IWYU pragma: associated

#include <Credential.pb.h>
#include <Enums.pb.h>
#include <cstdint>

#include "2_Factory.hpp"
#include "Proto.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Credential.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

namespace identity
{
namespace internal
{
class Authority;
}  // namespace internal

class Source;
}  // namespace identity

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template identity::credential::internal::Secondary* Factory::Credential(
    const api::Session&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const std::uint32_t,
    const crypto::Parameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Contact* Factory::Credential(
    const api::Session&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary& master,
    const std::uint32_t,
    const crypto::Parameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Verification* Factory::Credential(
    const api::Session&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary& master,
    const std::uint32_t,
    const crypto::Parameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Secondary* Factory::Credential(
    const api::Session&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole);
template identity::credential::internal::Contact* Factory::Credential(
    const api::Session&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole);
template identity::credential::internal::Verification* Factory::Credential(
    const api::Session&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole);

template <typename C>
struct deserialize_credential {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized) -> C*;
};
template <typename C>
struct make_credential {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const crypto::Parameters& nymParameter,
        const opentxs::PasswordPrompt& reason) -> C*;
};

template <>
struct deserialize_credential<identity::credential::internal::Contact> {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized)
        -> identity::credential::internal::Contact*
    {
        return opentxs::Factory::ContactCredential(
            api, parent, source, master, serialized);
    }
};
template <>
struct deserialize_credential<identity::credential::internal::Secondary> {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized)
        -> identity::credential::internal::Secondary*
    {
        return opentxs::Factory::SecondaryCredential(
            api, parent, source, master, serialized);
    }
};
template <>
struct deserialize_credential<identity::credential::internal::Verification> {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized)
        -> identity::credential::internal::Verification*
    {
        return opentxs::Factory::VerificationCredential(
            api, parent, source, master, serialized);
    }
};
template <>
struct make_credential<identity::credential::internal::Contact> {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const crypto::Parameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Contact*
    {
        return opentxs::Factory::ContactCredential(
            api, parent, source, master, nymParameters, version, reason);
    }
};
template <>
struct make_credential<identity::credential::internal::Secondary> {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const crypto::Parameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Secondary*
    {
        return opentxs::Factory::SecondaryCredential(
            api, parent, source, master, nymParameters, version, reason);
    }
};
template <>
struct make_credential<identity::credential::internal::Verification> {
    static auto Get(
        const api::Session& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const crypto::Parameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
        -> identity::credential::internal::Verification*
    {
        return opentxs::Factory::VerificationCredential(
            api, parent, source, master, nymParameters, version, reason);
    }
};

template <class C>
auto Factory::Credential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const std::uint32_t version,
    const crypto::Parameters& nymParameters,
    const proto::CredentialRole role,
    const opentxs::PasswordPrompt& reason) -> C*
{
    std::unique_ptr<C> output{make_credential<C>::Get(
        api, parent, source, master, version, nymParameters, reason)};

    if (!output) {
        LogError()("opentxs::Factory::")(__func__)(
            ":Failed to construct credential.")
            .Flush();

        return nullptr;
    }

    if (output->Role() != translate(role)) { return nullptr; }

    return output.release();
}

template <class C>
auto Factory::Credential(
    const api::Session& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const proto::Credential& serialized,
    const proto::KeyMode mode,
    const proto::CredentialRole role) -> C*
{
    // This check allows all constructors to assume inputs are well-formed
    if (!proto::Validate(serialized, VERBOSE, mode, role)) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Invalid serialized credential.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<C> output{deserialize_credential<C>::Get(
        api, parent, source, master, serialized)};

    if (false == bool(output)) { return nullptr; }

    if (output->Role() != translate(serialized.role())) { return nullptr; }

    if (false == output->Validate()) { return nullptr; }

    return output.release();
}
}  // namespace opentxs
