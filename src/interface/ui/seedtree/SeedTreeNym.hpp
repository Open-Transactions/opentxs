// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

class QVariant;

namespace opentxs::ui::implementation
{
using SeedTreeNymRow =
    Row<SeedTreeItemRowInternal,
        SeedTreeItemInternalInterface,
        SeedTreeItemRowID>;

class SeedTreeNym final : public SeedTreeNymRow
{
public:
    const api::session::Client& api_;

    auto NymID() const noexcept -> UnallocatedCString final
    {
        return row_id_.asBase58(api_.Crypto());
    }
    auto Index() const noexcept -> std::size_t final { return index_; }
    auto Name() const noexcept -> UnallocatedCString final;

    SeedTreeNym(
        const SeedTreeItemInternalInterface& parent,
        const api::session::Client& api,
        const SeedTreeItemRowID& rowID,
        const SeedTreeItemSortKey& sortKey,
        CustomData& custom) noexcept;
    SeedTreeNym() = delete;
    SeedTreeNym(const SeedTreeNym&) = delete;
    SeedTreeNym(SeedTreeNym&&) = delete;
    auto operator=(const SeedTreeNym&) -> SeedTreeNym& = delete;
    auto operator=(SeedTreeNym&&) -> SeedTreeNym& = delete;

    ~SeedTreeNym() override;

private:
    const SeedTreeItemSortKey index_;
    UnallocatedCString name_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const SeedTreeItemSortKey&, CustomData&) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation
