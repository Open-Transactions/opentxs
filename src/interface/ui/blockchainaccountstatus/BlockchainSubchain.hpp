// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/interface/ui/BlockchainSubchain.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

class QVariant;

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

namespace ui
{
class BlockchainSubchain;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using BlockchainSubchainRow =
    Row<BlockchainSubaccountRowInternal,
        BlockchainSubaccountInternalInterface,
        BlockchainSubaccountRowID>;

class BlockchainSubchain final : public BlockchainSubchainRow
{
public:
    const api::session::Client& api_;

    auto Name() const noexcept -> UnallocatedCString final;
    auto Progress() const noexcept -> UnallocatedCString final;
    auto Type() const noexcept -> blockchain::crypto::Subchain final
    {
        return row_id_;
    }

    BlockchainSubchain(
        const BlockchainSubaccountInternalInterface& parent,
        const api::session::Client& api,
        const BlockchainSubaccountRowID& rowID,
        const BlockchainSubaccountSortKey& sortKey,
        CustomData& custom) noexcept;
    BlockchainSubchain() = delete;
    BlockchainSubchain(const BlockchainSubchain&) = delete;
    BlockchainSubchain(BlockchainSubchain&&) = delete;
    auto operator=(const BlockchainSubchain&) -> BlockchainSubchain& = delete;
    auto operator=(BlockchainSubchain&&) -> BlockchainSubchain& = delete;

    ~BlockchainSubchain() final;

private:
    UnallocatedCString name_;
    UnallocatedCString progress_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const BlockchainSubaccountSortKey&, CustomData&) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSubchain>;
