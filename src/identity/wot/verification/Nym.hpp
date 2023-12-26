// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>

#include "internal/identity/wot/verification/Group.hpp"
#include "internal/identity/wot/verification/Item.hpp"
#include "internal/identity/wot/verification/Nym.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class Factory;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::implementation
{
class Nym final : public internal::Nym
{
public:
    operator SerializedType() const noexcept final;

    auto API() const noexcept -> const api::Session& final
    {
        return parent_.API();
    }
    /// Throws std::out_of_range for invalid position
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return *items_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto cbegin() const noexcept -> const_iterator final { return {this, 0}; }
    auto cend() const noexcept -> const_iterator final
    {
        return {this, items_.size()};
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ID() const noexcept -> const identifier::Nym& final { return id_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return parent_.External() ? id_ : parent_.NymID();
    }
    auto size() const noexcept -> std::size_t final { return items_.size(); }
    auto Version() const noexcept -> VersionNumber final { return version_; }

    auto AddItem(
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const verification::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept -> bool final;
    auto AddItem(const internal::Item::SerializedType item) noexcept
        -> bool final;
    auto DeleteItem(const identifier::Generic& item) noexcept -> bool final;
    auto UpgradeItemVersion(
        const VersionNumber itemVersion,
        VersionNumber& nymVersion) noexcept -> bool final;

    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;

    ~Nym() final = default;

private:
    friend opentxs::Factory;

    using Child = std::unique_ptr<internal::Item>;
    using Vector = UnallocatedVector<Child>;

    enum class Match { Accept, Reject, Replace };

    internal::Group& parent_;
    const VersionNumber version_;
    const identifier::Nym id_;
    Vector items_;

    static auto instantiate(
        internal::Nym& parent,
        const SerializedType& serialized) noexcept -> Vector;
    static auto match(
        const internal::Item& lhs,
        const internal::Item& rhs) noexcept -> Match;

    auto add_item(Child pCandidate) noexcept -> bool;

    Nym(internal::Group& parent,
        const identifier::Nym& nym,
        const VersionNumber version = DefaultVersion) noexcept;
    Nym(internal::Group& parent, const SerializedType& serialized) noexcept;
};
}  // namespace opentxs::identity::wot::verification::implementation
