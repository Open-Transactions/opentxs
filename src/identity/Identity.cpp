// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <utility>

#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/IdentityType.hpp"
#include "opentxs/identity/SourceProofType.hpp"
#include "opentxs/identity/SourceType.hpp"

namespace opentxs::identity
{
using namespace std::literals;

auto print(CredentialRole in) noexcept -> std::string_view
{
    using enum CredentialRole;
    static constexpr auto map =
        frozen::make_unordered_map<CredentialRole, std::string_view>({
            {MasterKey, "master"sv},
            {ChildKey, "key"sv},
            {Contact, "contact"sv},
            {Verify, "verification"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown identity::CredentialRole"sv;
    }
}

auto print(CredentialType in) noexcept -> std::string_view
{
    using enum CredentialType;
    static constexpr auto map =
        frozen::make_unordered_map<CredentialType, std::string_view>({
            {Legacy, "random key"sv},
            {HD, "deterministic key"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown identity::CredentialType"sv;
    }
}

auto print(Type in) noexcept -> std::string_view
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {individual, "individual"sv},
            {organization, "organization"sv},
            {business, "business"sv},
            {government, "government"sv},
            {server, "server"sv},
            {bot, "bot"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown identity::Type"sv;
    }
}

auto print(SourceProofType in) noexcept -> std::string_view
{
    using enum SourceProofType;
    static constexpr auto map =
        frozen::make_unordered_map<SourceProofType, std::string_view>({
            {SelfSignature, "self signature"sv},
            {Signature, "source signature"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown identity::SourceProofType"sv;
    }
}

auto print(SourceType in) noexcept -> std::string_view
{
    using enum SourceType;
    static constexpr auto map =
        frozen::make_unordered_map<SourceType, std::string_view>({
            {PubKey, "public key"sv},
            {Bip47, "payment code"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown identity::SourceType"sv;
    }
}
}  // namespace opentxs::identity
