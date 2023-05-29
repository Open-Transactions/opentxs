// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace verification
{
namespace internal
{
struct Item;
}  // namespace internal
}  // namespace verification
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::verification
{
class OPENTXS_EXPORT Item
{
public:
    enum class Type : bool { Confirm = true, Refute = false };
    enum class Validity : bool { Active = false, Retracted = true };

    static const VersionNumber DefaultVersion;

    virtual auto Begin() const noexcept -> Time = 0;
    virtual auto ClaimID() const noexcept -> const identifier::Generic& = 0;
    virtual auto End() const noexcept -> Time = 0;
    virtual auto ID() const noexcept -> const identifier::Generic& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Item& = 0;
    virtual auto Valid() const noexcept -> Validity = 0;
    virtual auto Value() const noexcept -> Type = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Item& = 0;

    Item(const Item&) = delete;
    Item(Item&&) = delete;
    auto operator=(const Item&) -> Item& = delete;
    auto operator=(Item&&) -> Item& = delete;

    virtual ~Item() = default;

protected:
    Item() = default;
};
}  // namespace opentxs::identity::wot::verification
