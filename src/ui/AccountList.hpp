// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/AccountList.hpp"

#include "internal/ui/UI.hpp"
#include "List.hpp"

namespace opentxs::ui::implementation
{
using AccountListList = List<
    AccountListExternalInterface,
    AccountListInternalInterface,
    AccountListRowID,
    AccountListRowInterface,
    AccountListRowInternal,
    AccountListRowBlank,
    AccountListSortKey,
    AccountListPrimaryID>;

class AccountList final : public AccountListList
{
public:
#if OT_QT
    // Data method needs this. And it needs to match the parent type for the Qt
    using qt_super =
        AccountListList::qt_super;  // class that appears below this one.
#endif

#if OT_QT
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
        noexcept final;
#endif
    ~AccountList();

private:
    friend api::client::implementation::UI;

    const ListenerDefinitions listeners_;

    void construct_row(
        const AccountListRowID& id,
        const AccountListSortKey& index,
        const CustomData& custom) const noexcept final;

    void process_account(const Identifier& id) noexcept;
    void process_account(const Identifier& id, const Amount balance) noexcept;
    void process_account(
        const Identifier& id,
        const Amount balance,
        const std::string& name) noexcept;
    void process_account(const network::zeromq::Message& message) noexcept;
    void startup() noexcept;

    AccountList(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback
#endif
        ) noexcept;
    AccountList() = delete;
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    AccountList& operator=(const AccountList&) = delete;
    AccountList& operator=(AccountList&&) = delete;
};
}  // namespace opentxs::ui::implementation
