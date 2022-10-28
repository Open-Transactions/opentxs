// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

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
class Group;
class Item;
}  // namespace verification
}  // namespace wot

class Nym;
}  // namespace identity

namespace proto
{
class VerificationSet;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification
{
class OPENTXS_EXPORT Set
{
public:
    using SerializedType = proto::VerificationSet;

    static const VersionNumber DefaultVersion;

    OPENTXS_NO_EXPORT virtual operator SerializedType() const noexcept = 0;

    virtual auto External() const noexcept -> const Group& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const Group& = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto AddItem(
        const identifier::Nym& claimOwner,
        const identifier::Generic& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value = Item::Type::Confirm,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = Item::DefaultVersion) noexcept
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept -> bool = 0;
    virtual auto DeleteItem(const identifier::Generic& item) noexcept
        -> bool = 0;
    virtual auto External() noexcept -> Group& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> Group& = 0;

    Set(const Set&) = delete;
    Set(Set&&) = delete;
    auto operator=(const Set&) -> Set& = delete;
    auto operator=(Set&&) -> Set& = delete;

    virtual ~Set() = default;

protected:
    Set() = default;
};
}  // namespace opentxs::identity::wot::verification
