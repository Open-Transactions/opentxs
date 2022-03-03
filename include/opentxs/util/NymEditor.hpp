// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
namespace imp
{
class Wallet;
}  // namespace imp

class Factory;
}  // namespace session
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

namespace identifier
{
class UnitDefinition;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace claim
{
class Data;
}  // namespace claim
}  // namespace wot
}  // namespace identity

namespace proto
{
class ContactData;
}  // namespace proto

class Identifier;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT NymData
{
public:
    NymData(const NymData&);
    NymData(NymData&&);

    auto asPublicNym() const -> identity::Nym::Serialized;
    auto BestEmail() const -> UnallocatedCString;
    auto BestPhoneNumber() const -> UnallocatedCString;
    auto BestSocialMediaProfile(
        const identity::wot::claim::ClaimType type) const -> UnallocatedCString;
    auto Claims() const -> const identity::wot::claim::Data&;
    auto DeleteClaim(const Identifier& id, const PasswordPrompt& reason)
        -> bool;
    auto EmailAddresses(bool active = true) const -> UnallocatedCString;
    auto HaveContract(
        const identifier::UnitDefinition& id,
        const UnitType currency,
        const bool primary,
        const bool active) const -> bool;
    auto Name() const -> UnallocatedCString;
    auto Nym() const -> const identity::Nym&;
    auto PaymentCode(const UnitType currency) const -> UnallocatedCString;
    auto PhoneNumbers(bool active = true) const -> UnallocatedCString;
    auto PreferredOTServer() const -> UnallocatedCString;
    auto PrintContactData() const -> UnallocatedCString;
    auto SocialMediaProfiles(
        const identity::wot::claim::ClaimType type,
        bool active = true) const -> UnallocatedCString;
    auto SocialMediaProfileTypes() const
        -> UnallocatedSet<identity::wot::claim::ClaimType>;
    auto Type() const -> identity::wot::claim::ClaimType;
    auto Valid() const -> bool;

    auto AddChildKeyCredential(
        const Identifier& strMasterID,
        const crypto::Parameters& nymParameters,
        const PasswordPrompt& reason) -> UnallocatedCString;
    auto AddClaim(const Claim& claim, const PasswordPrompt& reason) -> bool;
    auto AddContract(
        const UnallocatedCString& instrumentDefinitionID,
        const UnitType currency,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddEmail(
        const UnallocatedCString& value,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddPaymentCode(
        const UnallocatedCString& code,
        const UnitType currency,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddPhoneNumber(
        const UnallocatedCString& value,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    auto AddPreferredOTServer(
        const UnallocatedCString& id,
        const bool primary,
        const PasswordPrompt& reason) -> bool;
    auto AddSocialMediaProfile(
        const UnallocatedCString& value,
        const identity::wot::claim::ClaimType type,
        const bool primary,
        const bool active,
        const PasswordPrompt& reason) -> bool;
    void Release();
    auto SetCommonName(
        const UnallocatedCString& name,
        const PasswordPrompt& reason) -> bool;
    OPENTXS_NO_EXPORT auto SetContactData(
        const proto::ContactData& data,
        const PasswordPrompt& reason) -> bool;
    auto SetContactData(const ReadView& data, const PasswordPrompt& reason)
        -> bool;
    auto SetScope(
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& name,
        const bool primary,
        const PasswordPrompt& reason) -> bool;

    ~NymData();

private:
    friend api::session::imp::Wallet;

    using Lock = std::unique_lock<std::mutex>;
    using LockedSave = std::function<void(NymData*, Lock&)>;

    const api::session::Factory& factory_;
    std::unique_ptr<Lock> object_lock_;
    std::unique_ptr<LockedSave> locked_save_callback_;

    std::shared_ptr<identity::Nym> nym_;

    auto data() const -> const identity::wot::claim::Data&;

    auto nym() const -> const identity::Nym&;
    auto nym() -> identity::Nym&;

    void release();

    NymData(
        const api::session::Factory& factory,
        std::mutex& objectMutex,
        const std::shared_ptr<identity::Nym>& nym,
        LockedSave save);
    NymData() = delete;
    auto operator=(const NymData&) -> NymData& = delete;
    auto operator=(NymData&&) -> NymData& = delete;
};
}  // namespace opentxs
