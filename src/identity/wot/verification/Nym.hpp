// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <vector>

#include "internal/identity/wot/verification/Verification.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/identity/wot/verification/Nym.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

class Factory;
class Identifier;
class PasswordPrompt;
}  // namespace opentxs

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
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, items_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ID() const noexcept -> const identifier::Nym& final { return id_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return parent_.External() ? id_.get() : parent_.NymID();
    }
    auto size() const noexcept -> std::size_t final { return items_.size(); }
    auto Version() const noexcept -> VersionNumber final { return version_; }

    auto AddItem(
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept -> bool final;
    auto AddItem(const Item::SerializedType item) noexcept -> bool final;
    auto DeleteItem(const Identifier& item) noexcept -> bool final;
    auto UpgradeItemVersion(
        const VersionNumber itemVersion,
        VersionNumber& nymVersion) noexcept -> bool final;

    ~Nym() final = default;

private:
    friend opentxs::Factory;

    using Child = std::unique_ptr<internal::Item>;
    using Vector = std::pmr::vector<Child>;

    enum class Match { Accept, Reject, Replace };

    internal::Group& parent_;
    const VersionNumber version_;
    const OTNymID id_;
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
    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
