// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

class QVariant;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace ui
{
class BlockchainSubaccount;
}  // namespace ui

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using BlockchainSubaccountList = List<
    BlockchainSubaccountExternalInterface,
    BlockchainSubaccountInternalInterface,
    BlockchainSubaccountRowID,
    BlockchainSubaccountRowInterface,
    BlockchainSubaccountRowInternal,
    BlockchainSubaccountRowBlank,
    BlockchainSubaccountSortKey,
    BlockchainSubaccountPrimaryID>;
using BlockchainSubaccountRow = RowType<
    BlockchainSubaccountSourceRowInternal,
    BlockchainSubaccountSourceInternalInterface,
    BlockchainSubaccountSourceRowID>;

class BlockchainSubaccount final : public Combined<
                                       BlockchainSubaccountList,
                                       BlockchainSubaccountRow,
                                       BlockchainSubaccountSourceSortKey>
{
public:
    auto Name() const noexcept -> UnallocatedCString final { return key_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto SubaccountID() const noexcept -> const Identifier& final
    {
        return row_id_;
    }

    BlockchainSubaccount(
        const BlockchainSubaccountSourceInternalInterface& parent,
        const api::session::Client& api,
        const BlockchainSubaccountSourceRowID& rowID,
        const BlockchainSubaccountSourceSortKey& key,
        CustomData& custom) noexcept;
    ~BlockchainSubaccount() final;

private:
    auto construct_row(
        const BlockchainSubaccountRowID& id,
        const BlockchainSubaccountSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto last(const BlockchainSubaccountRowID& id) const noexcept -> bool final
    {
        return BlockchainSubaccountList::last(id);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(
        const implementation::BlockchainSubaccountSourceSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;

    BlockchainSubaccount() = delete;
    BlockchainSubaccount(const BlockchainSubaccount&) = delete;
    BlockchainSubaccount(BlockchainSubaccount&&) = delete;
    auto operator=(const BlockchainSubaccount&)
        -> BlockchainSubaccount& = delete;
    auto operator=(BlockchainSubaccount&&) -> BlockchainSubaccount& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSubaccount>;
