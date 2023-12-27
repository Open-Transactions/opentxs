// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "opentxs/Export.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
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
    static const VersionNumber DefaultVersion;

    virtual auto Begin() const noexcept -> Time = 0;
    virtual auto ClaimID() const noexcept -> const ClaimID& = 0;
    virtual auto End() const noexcept -> Time = 0;
    virtual auto ID() const noexcept -> const VerificationID& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Item& = 0;
    virtual auto Superscedes() const noexcept
        -> std::span<const VerificationID> = 0;
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
