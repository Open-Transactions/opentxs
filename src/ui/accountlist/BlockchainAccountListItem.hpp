// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <iosfwd>
#include <string>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/contract/Notary.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "ui/base/Row.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

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
class BlockchainAccountListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountListItemRow =
    Row<AccountListRowInternal, AccountListInternalInterface, AccountListRowID>;

class BlockchainAccountListItem final : public AccountListItemRow
{
public:
    auto AccountID() const noexcept -> std::string final
    {
        return row_id_->str();
    }
    auto Balance() const noexcept -> Amount final;
    auto ContractID() const noexcept -> std::string final { return contract_; }
    auto DisplayBalance() const noexcept -> std::string final
    {
        return blockchain::internal::Format(chain_, balance_);
    }
    auto DisplayUnit() const noexcept -> std::string final
    {
        return blockchain::internal::Ticker(chain_);
    }
    auto Name() const noexcept -> std::string final;
    auto NotaryID() const noexcept -> std::string final { return notary_; }
    auto NotaryName() const noexcept -> std::string final
    {
        return blockchain::DisplayString(chain_);
    }
    auto Type() const noexcept -> AccountType final { return type_; }
    auto Unit() const noexcept -> core::UnitType final { return unit_; }

    BlockchainAccountListItem(
        const AccountListInternalInterface& parent,
        const api::session::Client& api,
        const AccountListRowID& rowID,
        const AccountListSortKey& sortKey,
        CustomData& custom) noexcept;

    ~BlockchainAccountListItem() final = default;

private:
    const AccountType type_;
    const core::UnitType unit_;
    const blockchain::Type chain_;
    const std::string contract_;
    const std::string notary_;
    Amount balance_;
    std::string name_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const AccountListSortKey& key, CustomData& custom) noexcept
        -> bool final;

    BlockchainAccountListItem() = delete;
    BlockchainAccountListItem(const BlockchainAccountListItem&) = delete;
    BlockchainAccountListItem(BlockchainAccountListItem&&) = delete;
    auto operator=(const BlockchainAccountListItem&)
        -> BlockchainAccountListItem& = delete;
    auto operator=(BlockchainAccountListItem&&)
        -> BlockchainAccountListItem& = delete;
};
}  // namespace opentxs::ui::implementation
