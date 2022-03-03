// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <cstdint>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::proto
{
enum class ClaimType : bool {
    Indexed = true,
    Normal = false,
};
enum class VerificationType : bool {
    Indexed = true,
    Normal = false,
};

auto ContactAllowedContactData() noexcept -> const VersionMap&;
auto ContactDataAllowedContactSection() noexcept -> const VersionMap&;
auto ContactSectionAllowedItem() noexcept -> const VersionMap&;
auto VerificationAllowedSignature() noexcept -> const VersionMap&;
auto VerificationGroupAllowedIdentity() noexcept -> const VersionMap&;
auto VerificationIdentityAllowedVerification() noexcept -> const VersionMap&;
auto VerificationOfferAllowedClaim() noexcept -> const VersionMap&;
auto VerificationOfferAllowedVerification() noexcept -> const VersionMap&;
auto VerificationSetAllowedGroup() noexcept -> const VersionMap&;

auto ValidContactSectionName(
    const std::uint32_t version,
    const ContactSectionName name) -> bool;
auto ValidContactItemType(
    const ContactSectionVersion version,
    const ContactItemType itemType) -> bool;
auto ValidContactItemAttribute(
    const std::uint32_t version,
    const ContactItemAttribute attribute) -> bool;

auto TranslateSectionName(
    const std::uint32_t enumValue,
    const UnallocatedCString& lang = "en") -> UnallocatedCString;
auto TranslateItemType(
    const std::uint32_t enumValue,
    const UnallocatedCString& lang = "en") -> UnallocatedCString;
auto TranslateItemAttributes(
    const std::uint32_t enumValue,
    const UnallocatedCString& lang = "en") -> UnallocatedCString;
auto ReciprocalRelationship(const std::uint32_t relationship) -> std::uint32_t;
auto CheckCombination(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t version = 1) -> bool;
auto RequiredVersion(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t hint = 1) -> std::uint32_t;
auto NymRequiredVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint) -> std::uint32_t;
auto RequiredAuthorityVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint) -> std::uint32_t;
}  // namespace opentxs::proto
