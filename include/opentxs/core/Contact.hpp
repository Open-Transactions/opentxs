// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/core/PaymentCode.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"

#include <atomic>
#include <cstdint>
#include <ctime>
#include <memory>
#include <mutex>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace claim
{
class Data;
class Group;
class Item;
}  // namespace claim
}  // namespace wot

class Nym;
}  // namespace identity

namespace proto
{
class Contact;
class Nym;
}  // namespace proto

class ByteArray;
class Data;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT Contact
{
public:
    static auto Best(const identity::wot::claim::Group& group)
        -> std::shared_ptr<identity::wot::claim::Item>;
    static auto ExtractLabel(const identity::Nym& nym) -> UnallocatedCString;
    static auto ExtractType(const identity::Nym& nym)
        -> identity::wot::claim::ClaimType;
    static auto PaymentCode(
        const identity::wot::claim::Data& data,
        const UnitType currency) -> UnallocatedCString;

    OPENTXS_NO_EXPORT Contact(
        const api::session::Client& api,
        const proto::Contact& serialized);
    Contact(const api::session::Client& api, const UnallocatedCString& label);

    auto operator+=(Contact& rhs) -> Contact&;

    auto BestEmail() const -> UnallocatedCString;
    auto BestPhoneNumber() const -> UnallocatedCString;
    auto BestSocialMediaProfile(
        const identity::wot::claim::ClaimType type) const -> UnallocatedCString;
    auto BlockchainAddresses() const -> UnallocatedVector<std::tuple<
        ByteArray,
        blockchain::crypto::AddressStyle,
        blockchain::Type>>;
    auto Data() const -> std::shared_ptr<identity::wot::claim::Data>;
    auto EmailAddresses(bool active = true) const -> UnallocatedCString;
    auto ID() const -> const identifier::Generic&;
    auto Label() const -> const UnallocatedCString&;
    auto LastUpdated() const -> std::time_t;
    auto Nyms(const bool includeInactive = false) const
        -> UnallocatedVector<identifier::Nym>;
    auto PaymentCode(const UnitType currency = UnitType::Btc) const
        -> UnallocatedCString;
    auto PaymentCodes(const UnitType currency = UnitType::Btc) const
        -> UnallocatedVector<UnallocatedCString>;
    auto PaymentCodes(alloc::Default alloc) const -> Set<opentxs::PaymentCode>;
    auto PhoneNumbers(bool active = true) const -> UnallocatedCString;
    auto Print() const -> UnallocatedCString;
    OPENTXS_NO_EXPORT auto Serialize(proto::Contact& out) const -> bool;
    auto SocialMediaProfiles(
        const identity::wot::claim::ClaimType type,
        bool active = true) const -> UnallocatedCString;
    auto SocialMediaProfileTypes() const
        -> const UnallocatedSet<identity::wot::claim::ClaimType>;
    auto Type() const -> identity::wot::claim::ClaimType;

    auto AddBlockchainAddress(
        const UnallocatedCString& address,
        const blockchain::Type currency) -> bool;
    auto AddBlockchainAddress(
        const blockchain::crypto::AddressStyle& style,
        const blockchain::Type chain,
        const opentxs::Data& bytes) -> bool;
    auto AddEmail(
        const UnallocatedCString& value,
        const bool primary,
        const bool active) -> bool;
    auto AddNym(const Nym_p& nym, const bool primary) -> bool;
    auto AddNym(const identifier::Nym& nymID, const bool primary) -> bool;
    auto AddPaymentCode(
        const opentxs::PaymentCode& code,
        const bool primary,
        const UnitType currency = UnitType::Btc,
        const bool active = true) -> bool;
    auto AddPhoneNumber(
        const UnallocatedCString& value,
        const bool primary,
        const bool active) -> bool;
    auto AddSocialMediaProfile(
        const UnallocatedCString& value,
        const identity::wot::claim::ClaimType type,
        const bool primary,
        const bool active) -> bool;
    auto RemoveNym(const identifier::Nym& nymID) -> bool;
    void SetLabel(const UnallocatedCString& label);
    OPENTXS_NO_EXPORT void Update(const proto::Nym& nym);

    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    auto operator=(const Contact&) -> Contact& = delete;
    auto operator=(Contact&&) -> Contact& = delete;

    ~Contact();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs
