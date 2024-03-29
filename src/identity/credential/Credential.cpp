// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/credential/Base.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <functional>

#include "internal/identity/credential/Credential.hpp"
#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/Types.hpp"

namespace opentxs::identity::credential
{
using CredentialRoleMap =
    frozen::unordered_map<CredentialRole, protobuf::CredentialRole, 5>;
using CredentialRoleReverseMap =
    frozen::unordered_map<protobuf::CredentialRole, CredentialRole, 5>;
using CredentialTypeMap =
    frozen::unordered_map<CredentialType, protobuf::CredentialType, 3>;
using CredentialTypeReverseMap =
    frozen::unordered_map<protobuf::CredentialType, CredentialType, 3>;

auto credentialrole_map() noexcept -> const CredentialRoleMap&;
auto credentialtype_map() noexcept -> const CredentialTypeMap&;
}  // namespace opentxs::identity::credential

namespace opentxs::identity::credential
{
auto credentialrole_map() noexcept -> const CredentialRoleMap&
{
    using enum identity::CredentialRole;
    using enum protobuf::CredentialRole;
    static constexpr auto map = CredentialRoleMap{
        {Error, CREDROLE_ERROR},
        {MasterKey, CREDROLE_MASTERKEY},
        {ChildKey, CREDROLE_CHILDKEY},
        {Contact, CREDROLE_CONTACT},
        {Verify, CREDROLE_VERIFY},
    };

    return map;
}

auto credentialtype_map() noexcept -> const CredentialTypeMap&
{
    using enum identity::CredentialType;
    using enum protobuf::CredentialType;
    static constexpr auto map = CredentialTypeMap{
        {Error, CREDTYPE_ERROR},
        {HD, CREDTYPE_HD},
        {Legacy, CREDTYPE_LEGACY},
    };

    return map;
}
}  // namespace opentxs::identity::credential

namespace opentxs
{
auto translate(const identity::CredentialRole in) noexcept
    -> protobuf::CredentialRole
{
    try {
        return identity::credential::credentialrole_map().at(in);
    } catch (...) {
        return protobuf::CREDROLE_ERROR;
    }
}

auto translate(const identity::CredentialType in) noexcept
    -> protobuf::CredentialType
{
    try {
        return identity::credential::credentialtype_map().at(in);
    } catch (...) {
        return protobuf::CREDTYPE_ERROR;
    }
}

auto translate(const protobuf::CredentialRole in) noexcept
    -> identity::CredentialRole
{
    static const auto map = frozen::invert_unordered_map(
        identity::credential::credentialrole_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::CredentialRole::Error;
    }
}

auto translate(const protobuf::CredentialType in) noexcept
    -> identity::CredentialType
{
    static const auto map = frozen::invert_unordered_map(
        identity::credential::credentialtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::CredentialType::Error;
    }
}
}  // namespace opentxs
