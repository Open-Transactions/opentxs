// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/wot/verification/Nym.hpp"  // IWYU pragma: associated

#include <VerificationIdentity.pb.h>
#include <VerificationItem.pb.h>
#include <chrono>
#include <compare>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::VerificationNym(
    identity::wot::verification::internal::Group& parent,
    const identifier::Nym& nym,
    const VersionNumber version) -> identity::wot::verification::internal::Nym*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Nym;

    try {

        return new ReturnType(parent, nym, version);
    } catch (const std::exception& e) {
        LogError()()("Failed to construct verification nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::VerificationNym(
    identity::wot::verification::internal::Group& parent,
    const proto::VerificationIdentity& serialized)
    -> identity::wot::verification::internal::Nym*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Nym;

    try {

        return new ReturnType(parent, serialized);
    } catch (const std::exception& e) {
        LogError()()("Failed to construct verification nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::wot::verification
{
const VersionNumber Nym::DefaultVersion{1};
}

namespace opentxs::identity::wot::verification::implementation
{
Nym::Nym(
    internal::Group& parent,
    const identifier::Nym& nym,
    const VersionNumber version) noexcept
    : parent_(parent)
    , version_(version)
    , id_(nym)
    , items_()
{
}

Nym::Nym(internal::Group& parent, const SerializedType& in) noexcept
    : parent_(parent)
    , version_(in.version())
    , id_(parent_.API().Factory().Internal().NymID(in.nym()))
    , items_(instantiate(*this, in))
{
}

Nym::operator SerializedType() const noexcept
{
    const auto& api = API().Crypto();
    auto output = SerializedType{};
    output.set_version(version_);
    id_.Internal().Serialize(*output.mutable_nym());

    for (const auto& pItem : items_) {
        assert_false(nullptr == pItem);

        const auto& item = *pItem;
        output.add_verification()->CopyFrom(item.Serialize(api));
    }

    return output;
}

auto Nym::AddItem(
    const identifier::Generic& claim,
    const identity::Nym& signer,
    const PasswordPrompt& reason,
    const verification::Type value,
    const Time start,
    const Time end,
    const VersionNumber version) noexcept -> bool
{
    auto pCandidate = Child{Factory::VerificationItem(
        *this, claim, signer, reason, value, start, end, version, {})};

    if (false == bool(pCandidate)) {
        LogError()()("Failed to construct item").Flush();

        return false;
    }

    return add_item(std::move(pCandidate));
}

auto Nym::AddItem(const internal::Item::SerializedType item) noexcept -> bool
{
    auto pCandidate = Child{Factory::VerificationItem(*this, item)};

    if (false == bool(pCandidate)) {
        LogError()()("Failed to construct item").Flush();

        return false;
    }

    return add_item(std::move(pCandidate));
}

auto Nym::add_item(Child pCandidate) noexcept -> bool
{
    assert_false(nullptr == pCandidate);

    auto accept{true};
    const auto& candidate = *pCandidate;

    auto nymVersion{version_};

    if (false == UpgradeItemVersion(candidate.Version(), nymVersion)) {
        return false;
    }

    if (nymVersion != version_) {
        if (false == parent_.UpgradeNymVersion(nymVersion)) { return false; }

        const_cast<VersionNumber&>(version_) = nymVersion;
    }

    for (auto i{items_.cbegin()}; i != items_.cend();) {
        const auto& pItem = *i;

        assert_false(nullptr == pItem);

        const auto& item = *pItem;

        switch (match(candidate, item)) {
            case Match::Replace: {
                i = items_.erase(i);
            } break;
            case Match::Reject: {
                accept = false;
                [[fallthrough]];
            }
            case Match::Accept:
            default: {
                ++i;
            }
        }
    }

    if (accept) {
        parent_.Register(candidate.ID(), id_);
        items_.emplace_back(std::move(pCandidate));
    }

    return accept;
}

auto Nym::DeleteItem(const identifier::Generic& id) noexcept -> bool
{
    auto output{false};

    for (auto i{items_.cbegin()}; i != items_.cend(); ++i) {
        const auto& pItem = *i;

        assert_false(nullptr == pItem);

        const auto& item = *pItem;

        if (item.ID() == id) {
            output = true;
            items_.erase(i);
            parent_.Unregister(id);
            break;
        }
    }

    return output;
}

auto Nym::instantiate(internal::Nym& parent, const SerializedType& in) noexcept
    -> Vector
{
    auto output = Vector{};

    for (const auto& serialized : in.verification()) {
        auto pItem = std::unique_ptr<internal::Item>{
            Factory::VerificationItem(parent, serialized)};

        if (pItem) { output.emplace_back(std::move(pItem)); }
    }

    return output;
}

auto Nym::match(const internal::Item& lhs, const internal::Item& rhs) noexcept
    -> Match
{
    if (lhs.ClaimID() != rhs.ClaimID()) { return Match::Accept; }

    const auto subset =
        (lhs.Begin() >= rhs.Begin()) && (lhs.End() <= rhs.End());
    const auto superset =
        (lhs.Begin() <= rhs.Begin()) && (lhs.End() >= rhs.End());
    const auto overlapBegin =
        (lhs.Begin() >= rhs.Begin()) && (lhs.Begin() <= rhs.End());
    const auto overlapEnd =
        (lhs.End() >= rhs.Begin()) && (lhs.End() <= rhs.End());
    const auto overlap = overlapBegin || overlapEnd;

    if (subset) { return Match::Reject; }

    if (superset) { return Match::Replace; }

    if (false == overlap) { return Match::Accept; }

    if (lhs.Value() != rhs.Value()) { return Match::Reject; }

    return Match::Accept;
}

auto Nym::UpgradeItemVersion(
    const VersionNumber itemVersion,
    VersionNumber& nymVersion) noexcept -> bool
{
    try {
        while (true) {
            const auto [min, max] =
                proto::VerificationIdentityAllowedVerificationItem().at(
                    nymVersion);

            if (itemVersion < min) {
                LogError()()("Version ")(itemVersion)(" too old").Flush();

                return false;
            }

            if (itemVersion > max) {
                ++nymVersion;
            } else {

                return true;
            }
        }
    } catch (...) {
        LogError()()("No support for version ")(itemVersion)(" items").Flush();

        return false;
    }
}
}  // namespace opentxs::identity::wot::verification::implementation
