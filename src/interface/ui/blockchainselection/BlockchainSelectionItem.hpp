// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#pragma once

#include <ContactEnums.pb.h>
#include <atomic>
#include <iosfwd>
#include <memory>

#include "interface/ui/base/Row.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/interface/ui/BlockchainSelectionItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

class QVariant;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class BlockchainSelectionItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
namespace opentxs::ui::implementation
{
using BlockchainSelectionItemRow =
    Row<BlockchainSelectionRowInternal,
        BlockchainSelectionInternalInterface,
        BlockchainSelectionRowID>;

class BlockchainSelectionItem final
    : public BlockchainSelectionItemRow,
      public std::enable_shared_from_this<BlockchainSelectionItem>
{
public:
    const api::session::Client& api_;

    auto Name() const noexcept -> UnallocatedCString final { return name_; }
    auto IsEnabled() const noexcept -> bool final { return enabled_; }
    auto IsTestnet() const noexcept -> bool final { return testnet_; }
    auto Type() const noexcept -> blockchain::Type final { return row_id_; }

    BlockchainSelectionItem(
        const BlockchainSelectionInternalInterface& parent,
        const api::session::Client& api,
        const BlockchainSelectionRowID& rowID,
        const BlockchainSelectionSortKey& sortKey,
        CustomData& custom) noexcept;
    BlockchainSelectionItem() = delete;
    BlockchainSelectionItem(const BlockchainSelectionItem&) = delete;
    BlockchainSelectionItem(BlockchainSelectionItem&&) = delete;
    auto operator=(const BlockchainSelectionItem&)
        -> BlockchainSelectionItem& = delete;
    auto operator=(BlockchainSelectionItem&&)
        -> BlockchainSelectionItem& = delete;

    ~BlockchainSelectionItem() final;

private:
    const bool testnet_;
    const UnallocatedCString name_;
    std::atomic_bool enabled_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const BlockchainSelectionSortKey&, CustomData&) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation
#pragma GCC diagnostic pop

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSelectionItem>;
