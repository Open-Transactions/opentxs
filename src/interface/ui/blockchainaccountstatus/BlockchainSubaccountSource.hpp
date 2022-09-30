// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "interface/ui/blockchainaccountstatus/BlockchainSubaccount.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
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
class BlockchainSubaccountSource;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using BlockchainSubaccountSourceList = List<
    BlockchainSubaccountSourceExternalInterface,
    BlockchainSubaccountSourceInternalInterface,
    BlockchainSubaccountSourceRowID,
    BlockchainSubaccountSourceRowInterface,
    BlockchainSubaccountSourceRowInternal,
    BlockchainSubaccountSourceRowBlank,
    BlockchainSubaccountSourceSortKey,
    BlockchainSubaccountSourcePrimaryID>;
using BlockchainSubaccountSourceRow = RowType<
    BlockchainAccountStatusRowInternal,
    BlockchainAccountStatusInternalInterface,
    BlockchainAccountStatusRowID>;

class BlockchainSubaccountSource final : public Combined<
                                             BlockchainSubaccountSourceList,
                                             BlockchainSubaccountSourceRow,
                                             BlockchainAccountStatusSortKey>
{
public:
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto Name() const noexcept -> UnallocatedCString final
    {
        return key_.second;
    }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto SourceID() const noexcept -> const identifier::Generic& final
    {
        return row_id_;
    }
    auto Type() const noexcept -> blockchain::crypto::SubaccountType final
    {
        return key_.first;
    }

    BlockchainSubaccountSource(
        const BlockchainAccountStatusInternalInterface& parent,
        const api::session::Client& api,
        const BlockchainAccountStatusRowID& rowID,
        const BlockchainAccountStatusSortKey& key,
        CustomData& custom) noexcept;
    BlockchainSubaccountSource() = delete;
    BlockchainSubaccountSource(const BlockchainSubaccountSource&) = delete;
    BlockchainSubaccountSource(BlockchainSubaccountSource&&) = delete;
    auto operator=(const BlockchainSubaccountSource&)
        -> BlockchainSubaccountSource& = delete;
    auto operator=(BlockchainSubaccountSource&&)
        -> BlockchainSubaccountSource& = delete;

    ~BlockchainSubaccountSource() final;

private:
    auto construct_row(
        const BlockchainSubaccountSourceRowID& id,
        const BlockchainSubaccountSourceSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const BlockchainSubaccountSourceRowID& id) const noexcept
        -> bool final
    {
        return BlockchainSubaccountSourceList::last(id);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(
        const implementation::BlockchainAccountStatusSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSubaccountSource>;
