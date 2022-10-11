// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include "interface/ui/accountlist/AccountListItem.hpp"
#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/AccountListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
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
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class BlockchainAccountListItem final : public AccountListItem
{
public:
    auto NotaryName() const noexcept -> UnallocatedCString final
    {
        return notary_name_;
    }

    BlockchainAccountListItem(
        const AccountListInternalInterface& parent,
        const api::session::Client& api,
        const AccountListRowID& rowID,
        const AccountListSortKey& sortKey,
        CustomData& custom) noexcept;
    BlockchainAccountListItem() = delete;
    BlockchainAccountListItem(const BlockchainAccountListItem&) = delete;
    BlockchainAccountListItem(BlockchainAccountListItem&&) = delete;
    auto operator=(const BlockchainAccountListItem&)
        -> BlockchainAccountListItem& = delete;
    auto operator=(BlockchainAccountListItem&&)
        -> BlockchainAccountListItem& = delete;

    ~BlockchainAccountListItem() final;

private:
    const blockchain::Type chain_;
    const UnallocatedCString notary_name_;
};
}  // namespace opentxs::ui::implementation
