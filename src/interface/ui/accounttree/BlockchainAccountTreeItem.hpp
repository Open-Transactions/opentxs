// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "1_Internal.hpp"
#include "interface/ui/accounttree/AccountTreeItem.hpp"
#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/interface/ui/AccountTreeItem.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class BlockchainAccountTreeItem final : public AccountTreeItem
{
public:
    auto NotaryName() const noexcept -> UnallocatedCString final
    {
        return notary_name_;
    }

    BlockchainAccountTreeItem(
        const AccountCurrencyInternalInterface& parent,
        const api::session::Client& api,
        const AccountCurrencyRowID& rowID,
        const AccountCurrencySortKey& sortKey,
        CustomData& custom) noexcept;

    ~BlockchainAccountTreeItem() final;

private:
    const blockchain::Type chain_;
    const UnallocatedCString notary_name_;

    BlockchainAccountTreeItem() = delete;
    BlockchainAccountTreeItem(const BlockchainAccountTreeItem&) = delete;
    BlockchainAccountTreeItem(BlockchainAccountTreeItem&&) = delete;
    auto operator=(const BlockchainAccountTreeItem&)
        -> BlockchainAccountTreeItem& = delete;
    auto operator=(BlockchainAccountTreeItem&&)
        -> BlockchainAccountTreeItem& = delete;
};
}  // namespace opentxs::ui::implementation
