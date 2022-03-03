// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "opentxs/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class Signature;
}  // namespace proto

class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::implementation
{
class Signable : virtual public opentxs::contract::Signable
{
public:
    auto Alias() const noexcept -> UnallocatedCString override;
    auto ID() const noexcept -> OTIdentifier override;
    auto Nym() const noexcept -> Nym_p override;
    auto Terms() const noexcept -> const UnallocatedCString& override;
    auto Validate() const noexcept -> bool override;
    auto Version() const noexcept -> VersionNumber override;

    auto SetAlias(const UnallocatedCString& alias) noexcept -> bool override;

    ~Signable() override = default;

protected:
    using Signatures = UnallocatedList<Signature>;

    const api::Session& api_;
    mutable std::mutex lock_;
    const Nym_p nym_;
    const VersionNumber version_;
    const UnallocatedCString conditions_;
    const OTIdentifier id_;
    Signatures signatures_;
    UnallocatedCString alias_;

    auto CheckID(const Lock& lock) const -> bool;
    virtual auto id(const Lock& lock) const -> OTIdentifier;
    virtual auto validate(const Lock& lock) const -> bool = 0;
    virtual auto verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const -> bool;
    auto verify_write_lock(const Lock& lock) const -> bool;

    auto clear_signatures(const Lock& lock) noexcept -> void;
    virtual auto first_time_init(const Lock& lock) noexcept(false) -> void;
    virtual auto init_serialized(const Lock& lock) noexcept(false) -> void;
    virtual auto update_signature(
        const Lock& lock,
        const PasswordPrompt& reason) -> bool;
    auto update_version(const Lock& lock, const VersionNumber version) noexcept
        -> void;

    virtual auto GetID(const Lock& lock) const -> OTIdentifier = 0;

    Signable(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        const UnallocatedCString& conditions,
        const UnallocatedCString& alias) noexcept;
    Signable(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        const UnallocatedCString& conditions,
        const UnallocatedCString& alias,
        OTIdentifier&& id,
        Signatures&& signatures) noexcept;
    Signable(
        const api::Session& api,
        const Nym_p& nym,
        const VersionNumber version,
        const UnallocatedCString& conditions,
        const UnallocatedCString& alias,
        const Identifier& id,
        Signatures&& signatures) noexcept;
    Signable(const Signable&) noexcept;
    Signable(Signable&&) = delete;
    auto operator=(const Signable&) -> Signable& = delete;
    auto operator=(Signable&&) -> Signable& = delete;
};
}  // namespace opentxs::contract::implementation
