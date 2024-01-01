// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::ContactItemAttribute
// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_set_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "opentxs/protobuf/syntax/VerifyContacts.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_flat_set.hpp>
#include <opentxs/protobuf/ContactItemAttribute.pb.h>
#include <opentxs/protobuf/ContactItemType.pb.h>
#include <functional>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto ClaimAllowedContactItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 6}},
        {2, {1, 6}},
        {3, {1, 6}},
        {4, {1, 6}},
        {5, {1, 6}},
        {6, {1, 6}},
    };

    return output;
}
auto ClaimAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto ContactAllowedContactData() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 4}},
        {2, {5, 5}},
        {3, {6, 6}},
    };

    return output;
}
auto ContactDataAllowedContactSection() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 3}},
        {4, {4, 4}},
        {5, {5, 5}},
        {6, {6, 6}},
    };

    return output;
}
auto ContactItemAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 1}},
        {3, {1, 1}},
        {4, {1, 1}},
        {5, {1, 1}},
        {6, {1, 1}},
    };

    return output;
}
auto ContactSectionAllowedItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
        {2, {1, 2}},
        {3, {1, 3}},
        {4, {4, 4}},
        {5, {5, 5}},
        {6, {6, 6}},
    };

    return output;
}
auto VerificationAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationAllowedVerificationItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationIdentityAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationIdentityAllowedVerificationItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationItemAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationItemAllowedSignature() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationGroupAllowedIdentity() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationOfferAllowedClaim() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 6}},
    };

    return output;
}
auto VerificationOfferAllowedIdentifier() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationOfferAllowedVerificationItem() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto VerificationSetAllowedGroup() noexcept -> const VersionMap&
{
    static const auto output = VersionMap{
        {1, {1, 1}},
    };

    return output;
}
auto ValidContactSectionName(
    const std::uint32_t version,
    const ContactSectionName name) -> bool
{
    try {
        const auto& allowedNames = contact::AllowedSectionNames().at(version);

        return (std::ranges::find(allowedNames, name) != allowedNames.end());
    } catch (const std::out_of_range&) {
        return false;
    }
}

auto ValidContactItemType(
    const contact::ContactSectionVersion version,
    const ContactItemType itemType) -> bool
{
    try {
        const auto& allowedTypes = contact::AllowedItemTypes().at(version);

        return (
            std::ranges::find(allowedTypes, itemType) != allowedTypes.end());
    } catch (const std::out_of_range&) {
        return false;
    }
}
auto ValidContactItemAttribute(
    const std::uint32_t version,
    const ContactItemAttribute attribute) -> bool
{
    const auto allowedAttributes = contact::AllowedItemAttributes().at(version);

    try {
        return (
            std::ranges::find(allowedAttributes, attribute) !=
            allowedAttributes.end());
    } catch (const std::out_of_range&) {
        return false;
    }
}

auto TranslateSectionName(
    const std::uint32_t enumValue,
    const std::string_view lang) -> std::string_view
{
    const contact::EnumLang langPair{enumValue, lang};

    for (const auto& it : contact::ContactSectionNames()) {
        if (langPair == it.first) { return it.second; }
    }

    return {};
}
auto TranslateItemType(
    const std::uint32_t enumValue,
    const std::string_view lang) -> std::string_view
{
    const contact::EnumLang langPair{enumValue, lang};

    for (const auto& it : contact::ContactItemTypes()) {
        if (langPair == it.first) { return it.second; }
    }

    return {};
}
auto TranslateItemAttributes(
    const std::uint32_t enumValue,
    const std::string_view lang) -> std::string_view
{
    const contact::EnumLang langPair{enumValue, lang};

    for (const auto& it : contact::ContactItemAttributes()) {
        if (langPair == it.first) { return it.second; }
    }

    return {};
}

auto ReciprocalRelationship(const std::uint32_t relationship) -> std::uint32_t
{
    const auto& map = contact::RelationshipMap();
    auto input = static_cast<ContactItemType>(relationship);

    const bool found = (map.find(input) != map.end());

    if (found) {
        try {
            return static_cast<std::uint32_t>(map.at(input));
        } catch (const std::out_of_range&) {
        }
    }

    return CITEMTYPE_ERROR;
}

auto CheckCombination(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t version) -> bool
{
    const contact::ContactSectionVersion key{version, section};
    const auto it = contact::AllowedItemTypes().find(key);
    const bool keyExists = contact::AllowedItemTypes().end() != it;

    if (keyExists) {
        for (const auto& allowedType : it->second) {
            if (type == allowedType) { return true; }
        }
    }

    return false;
}

// 0 means not a valid combination for any version
auto RequiredVersion(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t hint) -> std::uint32_t
{
    for (std::uint32_t n = hint;
         n <= ContactSectionAllowedItem().rbegin()->first;
         ++n) {
        try {
            const auto exists =
                contact::AllowedItemTypes().at({n, section}).count(type);

            if (1 == exists) { return n; }
        } catch (const std::out_of_range&) {
        }
    }

    return 0;
}

auto NymRequiredVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint) -> std::uint32_t
{
    for (std::uint32_t n = hint;
         n <= ContactSectionAllowedItem().rbegin()->first;
         ++n) {
        try {
            const auto maxAuthority = NymAllowedAuthority().at(n).second;
            const auto maxCredential =
                AuthorityAllowedCredential().at(maxAuthority).second;
            const auto maxContactData =
                CredentialAllowedContactData().at(maxCredential).second;

            if (maxContactData >= contactDataVersion) { return n; }
        } catch (const std::out_of_range&) {

            return 0;
        }
    }

    return 0;
}

auto RequiredAuthorityVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint) -> std::uint32_t
{
    for (std::uint32_t n = hint;
         n <= ContactSectionAllowedItem().rbegin()->first;
         ++n) {
        try {
            const auto maxCredential =
                AuthorityAllowedCredential().at(n).second;
            const auto maxContactData =
                CredentialAllowedContactData().at(maxCredential).second;

            if (maxContactData >= contactDataVersion) { return n; }
        } catch (const std::out_of_range&) {
        }
    }

    return 0;
}
}  // namespace opentxs::protobuf::inline syntax
