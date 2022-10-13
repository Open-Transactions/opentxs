// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <utility>

#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Types.hpp"

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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto Currency() const noexcept -> UnitType final { return currency_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }

    AccountSummary(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback& cb) noexcept;
    AccountSummary() = delete;
    AccountSummary(const AccountSummary&) = delete;
    AccountSummary(AccountSummary&&) = delete;
    auto operator=(const AccountSummary&) -> AccountSummary& = delete;
    auto operator=(AccountSummary&&) -> AccountSummary& = delete;

    ~AccountSummary() final;

private:
    const ListenerDefinitions listeners_;
    const UnitType currency_;
    UnallocatedSet<identifier::Nym> issuers_;
    UnallocatedMap<identifier::Notary, identifier::Nym> server_issuer_map_;
    UnallocatedMap<identifier::Nym, identifier::Notary> nym_server_map_;

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
    void process_server(const identifier::Notary& serverID) noexcept;
    void startup() noexcept;
};
}  // namespace opentxs::ui::implementation
