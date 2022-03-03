// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identifier
{
class Nym;
}  // namespace identifier

namespace ui
{
class SeedTree;
class SeedTreeItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT SeedTree : virtual public List
{
public:
    virtual auto Debug() const noexcept -> UnallocatedCString = 0;
    virtual auto DefaultNym() const noexcept -> OTNymID = 0;
    virtual auto DefaultSeed() const noexcept -> OTIdentifier = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<SeedTreeItem> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<SeedTreeItem> = 0;

    ~SeedTree() override = default;

protected:
    SeedTree() noexcept = default;

private:
    SeedTree(const SeedTree&) = delete;
    SeedTree(SeedTree&&) = delete;
    auto operator=(const SeedTree&) -> SeedTree& = delete;
    auto operator=(SeedTree&&) -> SeedTree& = delete;
};
}  // namespace opentxs::ui
