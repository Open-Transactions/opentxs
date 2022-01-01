// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <list>
#include <map>
#include <set>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"

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
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountSummaryList = List<
    AccountSummaryExternalInterface,
    AccountSummaryInternalInterface,
    AccountSummaryRowID,
    AccountSummaryRowInterface,
    AccountSummaryRowInternal,
    AccountSummaryRowBlank,
    AccountSummarySortKey,
    AccountSummaryPrimaryID>;

class AccountSummary final : public AccountSummaryList
{
public:
    auto Currency() const noexcept -> core::UnitType final { return currency_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }

    AccountSummary(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const core::UnitType currency,
        const SimpleCallback& cb) noexcept;

    ~AccountSummary() final;

private:
    const ListenerDefinitions listeners_;
    const core::UnitType currency_;
    std::pmr::set<OTNymID> issuers_;
    std::pmr::map<OTServerID, OTNymID> server_issuer_map_;
    std::pmr::map<OTNymID, OTServerID> nym_server_map_;

    auto construct_row(
        const AccountSummaryRowID& id,
        const AccountSummarySortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto extract_key(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) noexcept -> AccountSummarySortKey;
    void process_connection(const Message& message) noexcept;
    void process_issuer(const identifier::Nym& issuerID) noexcept;
    void process_issuer(const Message& message) noexcept;
    void process_nym(const Message& message) noexcept;
    void process_server(const Message& message) noexcept;
    void process_server(const identifier::Server& serverID) noexcept;
    void startup() noexcept;

    AccountSummary() = delete;
    AccountSummary(const AccountSummary&) = delete;
    AccountSummary(AccountSummary&&) = delete;
    auto operator=(const AccountSummary&) -> AccountSummary& = delete;
    auto operator=(AccountSummary&&) -> AccountSummary& = delete;
};
}  // namespace opentxs::ui::implementation
