// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::UnitType

#pragma once

#include <atomic>
#include <iosfwd>
#include <memory>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/util/Container.hpp"

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

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

namespace ui
{
class IssuerItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <typename T>
struct make_blank;

template <>
struct make_blank<ui::implementation::IssuerItemRowID> {
    static auto value(const api::Session& api)
        -> ui::implementation::IssuerItemRowID
    {
        return {identifier::Generic{}, UnitType::Error};
    }
};
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using IssuerItemList = List<
    IssuerItemExternalInterface,
    IssuerItemInternalInterface,
    IssuerItemRowID,
    IssuerItemRowInterface,
    IssuerItemRowInternal,
    IssuerItemRowBlank,
    IssuerItemSortKey,
    IssuerItemPrimaryID>;
using IssuerItemRow = RowType<
    AccountSummaryRowInternal,
    AccountSummaryInternalInterface,
    AccountSummaryRowID>;

class IssuerItem final
    : public Combined<IssuerItemList, IssuerItemRow, AccountSummarySortKey>
{
public:
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ConnectionState() const noexcept -> bool final
    {
        return connection_.load();
    }
    auto Debug() const noexcept -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final;
    auto Trusted() const noexcept -> bool final { return issuer_->Paired(); }

    IssuerItem(
        const AccountSummaryInternalInterface& parent,
        const api::session::Client& api,
        const AccountSummaryRowID& rowID,
        const AccountSummarySortKey& sortKey,
        CustomData& custom,
        const UnitType currency) noexcept;
    IssuerItem() = delete;
    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    auto operator=(const IssuerItem&) -> IssuerItem& = delete;
    auto operator=(IssuerItem&&) -> IssuerItem& = delete;

    ~IssuerItem() final;

private:
    const ListenerDefinitions listeners_;
    const UnallocatedCString& name_;
    std::atomic<bool> connection_;
    const std::shared_ptr<const otx::client::Issuer> issuer_;
    const UnitType currency_;

    auto construct_row(
        const IssuerItemRowID& id,
        const IssuerItemSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto process_account(const identifier::Generic& accountID) noexcept -> void;
    auto process_account(const Message& message) noexcept -> void;
    auto refresh_accounts() noexcept -> void;
    auto reindex(const AccountSummarySortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup() noexcept -> void;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::IssuerItem>;
