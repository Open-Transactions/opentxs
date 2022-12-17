// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountsummary/AccountSummary.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>
#include <thread>
#include <utility>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/otx/client/Issuer.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/network/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto AccountSummaryModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const UnitType currency,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountSummary>
{
    using ReturnType = ui::implementation::AccountSummary;

    return std::make_unique<ReturnType>(api, nymID, currency, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
AccountSummary::AccountSummary(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const UnitType currency,
    const SimpleCallback& cb) noexcept
    : AccountSummaryList(api, nymID, cb, false)
    , api_(api)
    , listeners_({
          {api.Endpoints().IssuerUpdate().data(),
           new MessageProcessor<AccountSummary>(
               &AccountSummary::process_issuer)},
          {api.Endpoints().ServerUpdate().data(),
           new MessageProcessor<AccountSummary>(
               &AccountSummary::process_server)},
          {api.Endpoints().ConnectionStatus().data(),
           new MessageProcessor<AccountSummary>(
               &AccountSummary::process_connection)},
          {api.Endpoints().NymDownload().data(),
           new MessageProcessor<AccountSummary>(&AccountSummary::process_nym)},
      })
    , currency_{currency}
    , issuers_{}
    , server_issuer_map_{}
    , nym_server_map_{}
{
    setup_listeners(api, listeners_);
    startup_ = std::make_unique<std::thread>(&AccountSummary::startup, this);

    OT_ASSERT(startup_);
}

auto AccountSummary::construct_row(
    const AccountSummaryRowID& id,
    const AccountSummarySortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::IssuerItem(*this, api_, id, index, custom, currency_);
}

auto AccountSummary::extract_key(
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) noexcept -> AccountSummarySortKey
{
    AccountSummarySortKey output{false, "opentxs notary"};
    auto& [state, name] = output;

    const auto issuer = api_.Wallet().Internal().Issuer(nymID, issuerID);

    if (false == bool(issuer)) { return output; }

    const auto serverID = issuer->PrimaryServer();

    if (serverID.empty()) { return output; }

    try {
        const auto server = api_.Wallet().Internal().Server(serverID);
        name = server->Alias();
        const auto& serverNymID = server->Nym()->ID();
        eLock lock(shared_lock_);
        nym_server_map_.emplace(serverNymID, serverID);
        server_issuer_map_.emplace(serverID, issuerID);
    } catch (...) {
        return output;
    }

    switch (api_.ZMQ().Status(serverID.asBase58(api_.Crypto()))) {
        case network::ConnectionState::ACTIVE: {
            state = true;
        } break;
        case network::ConnectionState::NOT_ESTABLISHED:
        case network::ConnectionState::STALLED:
        default: {
        }
    }

    return output;
}

void AccountSummary::process_connection(const Message& message) noexcept
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto id = api_.Factory().NotaryIDFromHash(body.at(1).Bytes());
    process_server(id);
}

void AccountSummary::process_issuer(const identifier::Nym& issuerID) noexcept
{
    issuers_.emplace(issuerID);
    CustomData custom{};
    add_item(issuerID, extract_key(primary_id_, issuerID), custom);
}

void AccountSummary::process_issuer(const Message& message) noexcept
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto nymID = api_.Factory().NymIDFromHash(body.at(1).Bytes());
    const auto issuerID = api_.Factory().NymIDFromHash(body.at(2).Bytes());

    OT_ASSERT(false == nymID.empty());
    OT_ASSERT(false == issuerID.empty());

    if (nymID != primary_id_) { return; }

    process_issuer(issuerID);
}

void AccountSummary::process_nym(const Message& message) noexcept
{
    wait_for_startup();

    OT_ASSERT(1 < message.Body().size());

    const auto nymID = api_.Factory().NymIDFromHash(message.Body_at(1).Bytes());
    sLock lock(shared_lock_);
    const auto it = nym_server_map_.find(nymID);

    if (nym_server_map_.end() == it) { return; }

    const auto serverID = it->second;
    lock.unlock();

    process_server(serverID);
}

void AccountSummary::process_server(const Message& message) noexcept
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto serverID = api_.Factory().NotaryIDFromHash(body.at(1).Bytes());

    OT_ASSERT(false == serverID.empty());

    process_server(serverID);
}

void AccountSummary::process_server(const identifier::Notary& serverID) noexcept
{
    sLock lock(shared_lock_);
    const auto it = server_issuer_map_.find(serverID);

    if (server_issuer_map_.end() == it) { return; }

    const auto issuerID = it->second;
    lock.unlock();
    CustomData custom{};
    add_item(issuerID, extract_key(primary_id_, issuerID), custom);
}

void AccountSummary::startup() noexcept
{
    const auto issuers = api_.Wallet().IssuerList(primary_id_);
    LogDetail()(OT_PRETTY_CLASS())("Loading ")(issuers.size())(" issuers.")
        .Flush();

    for (const auto& id : issuers) { process_issuer(id); }

    finish_startup();
}

AccountSummary::~AccountSummary()
{
    for (const auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
