// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace verification
{
namespace internal
{
struct Set;
}  // namespace internal

class Group;
class Item;
}  // namespace verification
}  // namespace wot

class Nym;
}  // namespace identity

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification
{
class OPENTXS_EXPORT Set
{
public:
    static const VersionNumber DefaultVersion;

    virtual auto External() const noexcept -> const Group& = 0;
    virtual auto Internal() const noexcept -> const Group& = 0;
    OPENTXS_NO_EXPORT virtual auto Private() const noexcept
        -> const internal::Set& = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto AddItem(
        const identifier::Nym& claimOwner,
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const verification::Type value,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = Item::DefaultVersion) noexcept
        -> bool = 0;
    virtual auto DeleteItem(const identifier::Generic& item) noexcept
        -> bool = 0;
    virtual auto External() noexcept -> Group& = 0;
    virtual auto Internal() noexcept -> Group& = 0;
    OPENTXS_NO_EXPORT virtual auto Private() noexcept -> internal::Set& = 0;

    Set(const Set&) = delete;
    Set(Set&&) = delete;
    auto operator=(const Set&) -> Set& = delete;
    auto operator=(Set&&) -> Set& = delete;

    virtual ~Set() = default;

protected:
    Set() = default;
};
}  // namespace opentxs::identity::wot::verification
