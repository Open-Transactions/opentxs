// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/Signable.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::contract::implementation
{
Signable::Signable(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    const UnallocatedCString& conditions,
    const UnallocatedCString& alias,
    identifier::Generic&& id,
    Signatures&& signatures) noexcept
    : api_(api)
    , lock_()
    , nym_(nym)
    , version_(version)
    , conditions_(conditions)
    , id_(id)
    , signatures_(std::move(signatures))
    , alias_(alias)
{
}

Signable::Signable(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    const UnallocatedCString& conditions,
    const UnallocatedCString& alias,
    const identifier::Generic& id,
    Signatures&& signatures) noexcept
    : Signable(
          api,
          nym,
          version,
          conditions,
          alias,
          identifier::Generic{id},
          std::move(signatures))
{
}

Signable::Signable(
    const api::Session& api,
    const Nym_p& nym,
    const VersionNumber version,
    const UnallocatedCString& conditions,
    const UnallocatedCString& alias) noexcept
    : Signable(api, nym, version, conditions, alias, identifier::Generic{}, {})
{
}

Signable::Signable(const Signable& rhs) noexcept
    : api_(rhs.api_)
    , lock_()
    , nym_(rhs.nym_)
    , version_(rhs.version_)
    , conditions_(rhs.conditions_)
    , id_(rhs.id_)
    , signatures_(rhs.signatures_)
    , alias_(rhs.alias_)
{
}

auto Signable::Alias() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return alias_;
}

auto Signable::CheckID(const Lock& lock) const -> bool
{
    return (GetID(lock) == id_);
}

auto Signable::clear_signatures(const Lock& lock) noexcept -> void
{
    OT_ASSERT(verify_write_lock(lock));

    signatures_.clear();
}

auto Signable::first_time_init(const Lock& lock) -> void
{
    const_cast<identifier::Generic&>(id_) = GetID(lock);

    if (id_.empty()) { throw std::runtime_error("Failed to calculate id"); }
}

auto Signable::id(const Lock& lock) const -> identifier::Generic
{
    OT_ASSERT(verify_write_lock(lock));

    return id_;
}

auto Signable::ID() const noexcept -> identifier::Generic
{
    auto lock = Lock{lock_};

    return id(lock);
}

auto Signable::init_serialized(const Lock& lock) noexcept(false) -> void
{
    const auto id = GetID(lock);

    if (id_ != id) {
        const auto error = UnallocatedCString{"Calculated id ("} +
                           id.asBase58(api_.Crypto()) +
                           ") does not match serialized id (" +
                           id_.asBase58(api_.Crypto()) + ")";

        throw std::runtime_error(error);
    }
}

auto Signable::Nym() const noexcept -> Nym_p { return nym_; }

auto Signable::SetAlias(const UnallocatedCString& alias) noexcept -> bool
{
    auto lock = Lock{lock_};
    alias_ = alias;

    return true;
}

auto Signable::Terms() const noexcept -> const UnallocatedCString&
{
    auto lock = Lock{lock_};

    return conditions_;
}

auto Signable::update_signature(const Lock& lock, const PasswordPrompt& reason)
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    if (!nym_) {
        LogError()(OT_PRETTY_CLASS())("Missing nym.").Flush();

        return false;
    }

    return true;
}

auto Signable::update_version(
    const Lock& lock,
    const VersionNumber version) noexcept -> void
{
    clear_signatures(lock);
    const_cast<VersionNumber&>(version_) = version;
}

auto Signable::Validate() const noexcept -> bool
{
    auto lock = Lock{lock_};

    return validate(lock);
}

auto Signable::verify_write_lock(const Lock& lock) const -> bool
{
    if (lock.mutex() != &lock_) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(OT_PRETTY_CLASS())("Lock not owned.").Flush();

        return false;
    }

    return true;
}

auto Signable::verify_signature(const Lock& lock, const proto::Signature&) const
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    if (!nym_) {
        LogError()(OT_PRETTY_CLASS())("Missing nym.").Flush();

        return false;
    }

    return true;
}

auto Signable::Version() const noexcept -> VersionNumber { return version_; }
}  // namespace opentxs::contract::implementation
