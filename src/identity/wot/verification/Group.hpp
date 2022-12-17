// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>

#include "internal/identity/wot/verification/Verification.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification::implementation
{
class Group final : public internal::Group
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
        return *nyms_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto cbegin() const noexcept -> const_iterator final { return {this, 0}; }
    auto cend() const noexcept -> const_iterator final
    {
        return {this, nyms_.size()};
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto External() const noexcept -> bool final { return external_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return parent_.NymID();
    }
    auto size() const noexcept -> std::size_t final { return nyms_.size(); }
    auto UpgradeNymVersion(const VersionNumber version) noexcept -> bool final;
    auto Version() const noexcept -> VersionNumber final { return version_; }

    auto AddItem(
        const identifier::Nym& claimOwner,
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept -> bool final;
    auto AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept -> bool final;
    /// Throws std::out_of_range for invalid position
    auto at(const std::size_t position) noexcept(false) -> value_type& final
    {
        return *nyms_.at(position);
    }
    auto begin() noexcept -> iterator final { return {this, 0}; }
    auto DeleteItem(const identifier::Generic& item) noexcept -> bool final;
    auto end() noexcept -> iterator final { return {this, nyms_.size()}; }
    void Register(
        const identifier::Generic& id,
        const identifier::Nym& nym) noexcept final;
    void Unregister(const identifier::Generic& id) noexcept final;

    Group() = delete;
    Group(const Group&) = delete;
    Group(Group&&) = delete;
    auto operator=(const Group&) -> Group& = delete;
    auto operator=(Group&&) -> Group& = delete;

    ~Group() final = default;

private:
    friend opentxs::Factory;

    using Vector = UnallocatedVector<std::unique_ptr<internal::Nym>>;

    internal::Set& parent_;
    const VersionNumber version_;
    const bool external_;
    Vector nyms_;
    UnallocatedMap<identifier::Generic, identifier::Nym> map_;

    static auto instantiate(
        internal::Group& parent,
        const SerializedType& serialized) noexcept -> Vector;

    auto get_nym(const identifier::Nym& nym) noexcept -> internal::Nym&;

    Group(
        internal::Set& parent,
        bool external,
        const VersionNumber version = DefaultVersion) noexcept;
    Group(
        internal::Set& parent,
        const SerializedType& serialized,
        bool external) noexcept;
};
}  // namespace opentxs::identity::wot::verification::implementation
