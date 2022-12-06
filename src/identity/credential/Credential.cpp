// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::CredentialRole
// IWYU pragma: no_forward_declare opentxs::identity::CredentialType

#include "identity/credential/Base.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <robin_hood.h>

#include "internal/identity/credential/Credential.hpp"
#include "util/Container.hpp"

namespace opentxs::identity::credential
{
using CredentialRoleMap =
    robin_hood::unordered_flat_map<CredentialRole, proto::CredentialRole>;
using CredentialRoleReverseMap =
    robin_hood::unordered_flat_map<proto::CredentialRole, CredentialRole>;
using CredentialTypeMap =
    robin_hood::unordered_flat_map<CredentialType, proto::CredentialType>;
using CredentialTypeReverseMap =
    robin_hood::unordered_flat_map<proto::CredentialType, CredentialType>;

auto credentialrole_map() noexcept -> const CredentialRoleMap&;
auto credentialtype_map() noexcept -> const CredentialTypeMap&;
}  // namespace opentxs::identity::credential

namespace opentxs::identity::credential
{
auto credentialrole_map() noexcept -> const CredentialRoleMap&
{
    static const auto map = CredentialRoleMap{
        {identity::CredentialRole::Error, proto::CREDROLE_ERROR},
        {identity::CredentialRole::MasterKey, proto::CREDROLE_MASTERKEY},
        {identity::CredentialRole::ChildKey, proto::CREDROLE_CHILDKEY},
        {identity::CredentialRole::Contact, proto::CREDROLE_CONTACT},
        {identity::CredentialRole::Verify, proto::CREDROLE_VERIFY},
    };

    return map;
}

auto credentialtype_map() noexcept -> const CredentialTypeMap&
{
    static const auto map = CredentialTypeMap{
        {identity::CredentialType::Error, proto::CREDTYPE_ERROR},
        {identity::CredentialType::HD, proto::CREDTYPE_HD},
        {identity::CredentialType::Legacy, proto::CREDTYPE_LEGACY},
    };

    return map;
}
}  // namespace opentxs::identity::credential

namespace opentxs
{
auto translate(const identity::CredentialRole in) noexcept
    -> proto::CredentialRole
{
    try {
        return identity::credential::credentialrole_map().at(in);
    } catch (...) {
        return proto::CREDROLE_ERROR;
    }
}

auto translate(const identity::CredentialType in) noexcept
    -> proto::CredentialType
{
    try {
        return identity::credential::credentialtype_map().at(in);
    } catch (...) {
        return proto::CREDTYPE_ERROR;
    }
}

auto translate(const proto::CredentialRole in) noexcept
    -> identity::CredentialRole
{
    static const auto map = reverse_arbitrary_map<
        identity::CredentialRole,
        proto::CredentialRole,
        identity::credential::CredentialRoleReverseMap>(
        identity::credential::credentialrole_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::CredentialRole::Error;
    }
}

auto translate(const proto::CredentialType in) noexcept
    -> identity::CredentialType
{
    static const auto map = reverse_arbitrary_map<
        identity::CredentialType,
        proto::CredentialType,
        identity::credential::CredentialTypeReverseMap>(
        identity::credential::credentialtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::CredentialType::Error;
    }
}
}  // namespace opentxs
