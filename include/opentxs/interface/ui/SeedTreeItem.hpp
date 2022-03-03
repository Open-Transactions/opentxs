// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class SeedTreeNym;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT SeedTreeItem : virtual public List, virtual public ListRow
{
public:
    virtual auto Debug() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept -> SharedPimpl<SeedTreeNym> = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept -> SharedPimpl<SeedTreeNym> = 0;
    virtual auto SeedID() const noexcept -> UnallocatedCString = 0;
    virtual auto Type() const noexcept -> crypto::SeedStyle = 0;

    ~SeedTreeItem() override = default;

protected:
    SeedTreeItem() noexcept = default;

private:
    SeedTreeItem(const SeedTreeItem&) = delete;
    SeedTreeItem(SeedTreeItem&&) = delete;
    auto operator=(const SeedTreeItem&) -> SeedTreeItem& = delete;
    auto operator=(SeedTreeItem&&) -> SeedTreeItem& = delete;
};
}  // namespace opentxs::ui
