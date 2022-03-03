// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "opentxs/identity/Types.hpp"  // IWYU pragma: associated

#include <robin_hood.h>

#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/SourceProofType.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto print(identity::CredentialRole in) noexcept -> std::string_view
{
    static const auto map = robin_hood::
        unordered_flat_map<identity::CredentialRole, UnallocatedCString>{
            {identity::CredentialRole::MasterKey, "master"},
            {identity::CredentialRole::ChildKey, "key"},
            {identity::CredentialRole::Contact, "contact"},
            {identity::CredentialRole::Verify, "verification"},
        };

    try {
        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}

auto print(identity::CredentialType in) noexcept -> std::string_view
{
    static const auto map = robin_hood::
        unordered_flat_map<identity::CredentialType, UnallocatedCString>{
            {identity::CredentialType::Legacy, "random key"},
            {identity::CredentialType::HD, "deterministic key"},
        };

    try {
        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}

auto print(identity::Type in) noexcept -> std::string_view
{
    static const auto map =
        robin_hood::unordered_flat_map<identity::Type, UnallocatedCString>{
            {identity::Type::individual, "individual"},
            {identity::Type::organization, "organization"},
            {identity::Type::business, "business"},
            {identity::Type::government, "government"},
            {identity::Type::server, "server"},
            {identity::Type::bot, "bot"},
        };

    try {
        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}

auto print(identity::SourceProofType in) noexcept -> std::string_view
{
    static const auto map = robin_hood::
        unordered_flat_map<identity::SourceProofType, UnallocatedCString>{
            {identity::SourceProofType::SelfSignature, "self signature"},
            {identity::SourceProofType::Signature, "source signature"},
        };

    try {
        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}

auto print(identity::SourceType in) noexcept -> std::string_view
{
    static const auto map = robin_hood::
        unordered_flat_map<identity::SourceType, UnallocatedCString>{
            {identity::SourceType::PubKey, "public key"},
            {identity::SourceType::Bip47, "payment code"},
        };

    try {
        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}
}  // namespace opentxs
