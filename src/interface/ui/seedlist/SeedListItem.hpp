// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api
}  // namespace opentxs

class QVariant;
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using SeedListItemRow =
    Row<SeedListRowInternal, SeedListInternalInterface, SeedListRowID>;

class SeedListItem final : public SeedListItemRow
{
public:
    const api::session::Client& api_;

    auto SeedID() const noexcept -> const crypto::SeedID& final
    {
        return row_id_;
    }
    auto Name() const noexcept -> UnallocatedCString final;
    auto Type() const noexcept -> crypto::SeedStyle final { return type_; }

    SeedListItem(
        const SeedListInternalInterface& parent,
        const api::session::Client& api,
        const SeedListRowID& rowID,
        const SeedListSortKey& sortKey,
        CustomData& custom) noexcept;
    SeedListItem() = delete;
    SeedListItem(const SeedListItem&) = delete;
    SeedListItem(SeedListItem&&) = delete;
    auto operator=(const SeedListItem&) -> SeedListItem& = delete;
    auto operator=(SeedListItem&&) -> SeedListItem& = delete;

    ~SeedListItem() override;

private:
    const crypto::SeedStyle type_;
    UnallocatedCString name_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const SeedListSortKey&, CustomData&) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation
