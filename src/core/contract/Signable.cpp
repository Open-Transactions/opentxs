// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/Signable.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#define OT_METHOD "opentxs::Signable::"

namespace opentxs
{
Signable::Signable(const ConstNym& nym)
    : alias_()
    , id_(Identifier::Factory())
    , nym_(nym)
    , signatures_()
    , version_()
    , conditions_()
    , lock_()
{
}

Signable::Signable(const ConstNym& nym, const std::uint32_t version)
    : alias_()
    , id_(Identifier::Factory())
    , nym_(nym)
    , signatures_()
    , version_(version)
    , conditions_()
    , lock_()
{
}

Signable::Signable(
    const ConstNym& nym,
    const std::uint32_t version,
    const std::string& conditions)
    : alias_()
    , id_(Identifier::Factory())
    , nym_(nym)
    , signatures_()
    , version_(version)
    , conditions_(conditions)
    , lock_()
{
}

std::string Signable::Alias() const
{
    Lock lock(lock_);

    return alias_;
}

bool Signable::CalculateID(const Lock& lock)
{
    id_ = Identifier::Factory(GetID(lock));

    return true;
}

bool Signable::CheckID(const Lock& lock) const { return (GetID(lock) == id_); }

OTIdentifier Signable::id(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    return id_;
}

OTIdentifier Signable::ID() const
{
    Lock lock(lock_);

    return id(lock);
}

ConstNym Signable::Nym() const { return nym_; }

void Signable::SetAlias(const std::string& alias)
{
    Lock lock(lock_);

    alias_ = alias;
}

const std::string& Signable::Terms() const
{
    Lock lock(lock_);

    return conditions_;
}

bool Signable::update_signature(const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    if (!nym_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing nym.").Flush();

        return false;
    }

    return true;
}

bool Signable::Validate() const
{
    Lock lock(lock_);

    return validate(lock);
}

bool Signable::verify_write_lock(const Lock& lock) const
{
    if (lock.mutex() != &lock_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock not owned.").Flush();

        return false;
    }

    return true;
}

bool Signable::verify_signature(const Lock& lock, const proto::Signature&) const
{
    OT_ASSERT(verify_write_lock(lock));

    if (!nym_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing nym.").Flush();

        return false;
    }

    return true;
}

const std::uint32_t& Signable::Version() const { return version_; }
}  // namespace opentxs
