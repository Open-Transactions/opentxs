// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Endpoints
// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "api/session/Endpoints.hpp"  // IWYU pragma: associated

#include <functional>
#include <memory>
#include <span>
#include <utility>

#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/internal.factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto EndpointsAPI(const api::Crypto& api, const int instance) noexcept
    -> std::unique_ptr<api::session::Endpoints>
{
    using ReturnType = api::session::imp::Endpoints;

    return std::make_unique<ReturnType>(api, instance);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::internal
{
auto Endpoints::Asio() noexcept -> std::string_view
{
    static const auto endpoint =
        opentxs::network::zeromq::MakeDeterministicInproc("asio", -1, 1);

    return endpoint;
}

auto Endpoints::ContextShutdown() noexcept -> std::string_view
{
    static const auto endpoint =
        opentxs::network::zeromq::MakeDeterministicInproc("shutdown", -1, 1);

    return endpoint;
}
}  // namespace opentxs::api::session::internal

namespace opentxs::api::session::imp
{
Endpoints::Endpoints(const api::Crypto& api, const int instance) noexcept
    : crypto_(api)
    , instance_(instance)
    , account_update_(build_inproc_path("accountupdate", version_1_))
    , blockchain_account_created_(
          build_inproc_path("blockchain/account/new", version_1_))
    , blockchain_balance_(
          build_inproc_path("blockchain/balance/interactive", version_1_))
    , blockchain_block_available_(
          build_inproc_path("blockchain/block/available", version_1_))
    , blockchain_block_download_queue_(
          build_inproc_path("blockchain/block/queue", version_1_))
    , blockchain_mempool_(build_inproc_path("blockchain/mempool", version_1_))
    , blockchain_message_router_(
          opentxs::network::zeromq::MakeArbitraryInproc(alloc::Default{}))
    , blockchain_oracle_progress_(
          build_inproc_path("blockchain/block/tip", version_1_))
    , blockchain_new_filter_(build_inproc_path("blockchain/filter", version_1_))
    , blockchain_peer_(build_inproc_path("blockchain/peer/active", version_1_))
    , blockchain_peer_connection_(
          build_inproc_path("blockchain/peer/connected", version_1_))
    , blockchain_reorg_(build_inproc_path("blockchain/reorg", version_1_))
    , blockchain_report_status_(
          build_inproc_path("blockchain/report_status", version_1_))
    , blockchain_scan_progress_(
          build_inproc_path("blockchain/scan", version_1_))
    , blockchain_startup_publish_(
          build_inproc_path("blockchain/startup/publish", version_1_))
    , blockchain_startup_pull_(
          build_inproc_path("blockchain/startup/pull", version_1_))
    , blockchain_state_change_(
          build_inproc_path("blockchain/state", version_1_))
    , blockchain_sync_checksum_failure_(
          opentxs::network::zeromq::MakeArbitraryInproc(alloc::Default{}))
    , blockchain_sync_progress_(
          build_inproc_path("blockchain/sync", version_1_))
    , blockchain_sync_server_progress_(
          build_inproc_path("blockchain/sync/indexer", version_1_))
    , blockchain_server_updated_(
          build_inproc_path("blockchain/sync/db", version_1_))
    , blockchain_transactions_(
          build_inproc_path("blockchain/transactions", version_1_))
    , blockchain_wallet_updated_(
          build_inproc_path("blockchain/balance", version_1_))
    , connection_status_(build_inproc_path("connectionstatus", version_1_))
    , contact_update_(build_inproc_path("contactupdate", version_1_))
    , find_nym_(build_inproc_path("otx/search/nym", version_1_))
    , find_server_(build_inproc_path("otx/search/server", version_1_))
    , find_unit_definition_(build_inproc_path("otx/search/unit", version_1_))
    , issuer_update_(build_inproc_path("issuerupdate", version_1_))
    , messagability_(build_inproc_path("otx/messagability", version_1_))
    , message_loaded_(build_inproc_path("otx/message_loaded", version_1_))
    , nym_created_(build_inproc_path("nymcreated", version_1_))
    , nym_download_(build_inproc_path("nymupdate", version_1_))
    , otdht_blockchain_([&] {
        auto out = BlockchainMap{};

        for (const auto& chain : opentxs::blockchain::defined_chains()) {
            out.emplace(
                chain,
                build_inproc_path(
                    "internal/otdht/blockchain", version_1_, print(chain)));
        }

        return out;
    }())
    , otdht_node_router_(
          build_inproc_path("internal/otdht/node/router", version_1_))
    , otdht_node_publish_(
          build_inproc_path("internal/otdht/node/publish", version_1_))
    , otdht_node_pull_(
          build_inproc_path("internal/otdht/node/pull", version_1_))
    , otdht_wallet_(build_inproc_path("internal/otdht/wallet", version_1_))
    , pair_event_(build_inproc_path("pairevent", version_1_))
    , peer_reply_new_(build_inproc_path("peer/reply", version_1_))
    , peer_reply_update_(build_inproc_path("peerreplyupdate", version_1_))
    , peer_request_new_(build_inproc_path("peer/request", version_1_))
    , peer_request_update_(build_inproc_path("peerrequestupdate", version_1_))
    , pending_bailment_(
          build_inproc_path("peerrequest/pendingbailment", version_1_))
    , process_push_notification_(
          opentxs::network::zeromq::MakeArbitraryInproc())
    , push_notification_(opentxs::network::zeromq::MakeArbitraryInproc())
    , seed_updated_(build_inproc_path("crypto/seed", version_1_))
    , server_reply_received_(build_inproc_path("reply/received", version_1_))
    , server_request_sent_(build_inproc_path("request/sent", version_1_))
    , server_update_(build_inproc_path("serverupdate", version_1_))
    , shutdown_(build_inproc_path("shutdown", version_1_))
    , task_complete_(build_inproc_path("taskcomplete", version_1_))
    , unit_update_(build_inproc_path("unitupdate", version_1_))
    , widget_update_(build_inproc_path("ui/widgetupdate", version_1_))
    , workflow_account_update_(
          build_inproc_path("ui/workflowupdate/account", version_1_))
    , blockchain_transactions_map_()
    , thread_map_()
{
}

auto Endpoints::build_inproc_path(
    const std::string_view path,
    const int version) const noexcept -> CString
{
    return opentxs::network::zeromq::MakeDeterministicInproc(
               path, instance_, version)
        .c_str();
}

auto Endpoints::build_inproc_path(
    const std::string_view path,
    const int version,
    const std::string_view suffix) const noexcept -> CString
{
    return opentxs::network::zeromq::MakeDeterministicInproc(
               path, instance_, version, suffix)
        .c_str();
}

auto Endpoints::AccountUpdate() const noexcept -> std::string_view
{
    return account_update_;
}

auto Endpoints::BlockchainAccountCreated() const noexcept -> std::string_view
{
    return blockchain_account_created_;
}

auto Endpoints::BlockchainBalance() const noexcept -> std::string_view
{
    return blockchain_balance_;
}

auto Endpoints::BlockchainBlockAvailable() const noexcept -> std::string_view
{
    return blockchain_block_available_;
}

auto Endpoints::BlockchainBlockDownloadQueue() const noexcept
    -> std::string_view
{
    return blockchain_block_download_queue_;
}

auto Endpoints::BlockchainBlockOracleProgress() const noexcept
    -> std::string_view
{
    return blockchain_oracle_progress_;
}

auto Endpoints::BlockchainMempool() const noexcept -> std::string_view
{
    return blockchain_mempool_;
}

auto Endpoints::BlockchainMessageRouter() const noexcept -> std::string_view
{
    return blockchain_message_router_;
}

auto Endpoints::BlockchainNewFilter() const noexcept -> std::string_view
{
    return blockchain_new_filter_;
}

auto Endpoints::BlockchainPeer() const noexcept -> std::string_view
{
    return blockchain_peer_;
}

auto Endpoints::BlockchainPeerConnection() const noexcept -> std::string_view
{
    return blockchain_peer_connection_;
}

auto Endpoints::BlockchainReorg() const noexcept -> std::string_view
{
    return blockchain_reorg_;
}

auto Endpoints::BlockchainReportStatus() const noexcept -> std::string_view
{
    return blockchain_report_status_;
}

auto Endpoints::BlockchainScanProgress() const noexcept -> std::string_view
{
    return blockchain_scan_progress_;
}

auto Endpoints::BlockchainStartupPublish() const noexcept -> std::string_view
{
    return blockchain_startup_publish_;
}

auto Endpoints::BlockchainStartupPull() const noexcept -> std::string_view
{
    return blockchain_startup_pull_;
}

auto Endpoints::BlockchainStateChange() const noexcept -> std::string_view
{
    return blockchain_state_change_;
}

auto Endpoints::BlockchainSyncChecksumFailure() const noexcept
    -> std::string_view
{
    return blockchain_sync_checksum_failure_;
}

auto Endpoints::BlockchainSyncProgress() const noexcept -> std::string_view
{
    return blockchain_sync_progress_;
}

auto Endpoints::BlockchainSyncServerProgress() const noexcept
    -> std::string_view
{
    return blockchain_sync_server_progress_;
}

auto Endpoints::BlockchainSyncServerUpdated() const noexcept -> std::string_view
{
    return blockchain_server_updated_;
}

auto Endpoints::BlockchainTransactions() const noexcept -> std::string_view
{
    return blockchain_transactions_;
}

auto Endpoints::BlockchainTransactions(
    const identifier::Nym& nym) const noexcept -> std::string_view
{
    auto handle = blockchain_transactions_map_.lock();

    if (auto it = handle->find(nym); handle->end() != it) {

        return it->second;
    } else {
        static constexpr auto prefix{"blockchain/transactions"};
        const auto path =
            CString{prefix} + "by_nym/" + nym.asBase58(crypto_).c_str();
        auto [out, added] =
            handle->try_emplace(nym, build_inproc_path(path, version_1_));

        return out->second;
    }
}

auto Endpoints::BlockchainWalletUpdated() const noexcept -> std::string_view
{
    return blockchain_wallet_updated_;
}

auto Endpoints::ConnectionStatus() const noexcept -> std::string_view
{
    return connection_status_;
}

auto Endpoints::ContactUpdate() const noexcept -> std::string_view
{
    return contact_update_;
}

auto Endpoints::FindNym() const noexcept -> std::string_view
{
    return find_nym_;
}

auto Endpoints::FindServer() const noexcept -> std::string_view
{
    return find_server_;
}

auto Endpoints::FindUnitDefinition() const noexcept -> std::string_view
{
    return find_unit_definition_;
}

auto Endpoints::IssuerUpdate() const noexcept -> std::string_view
{
    return issuer_update_;
}

auto Endpoints::Messagability() const noexcept -> std::string_view
{
    return messagability_;
}

auto Endpoints::MessageLoaded() const noexcept -> std::string_view
{
    return message_loaded_;
}

auto Endpoints::NymCreated() const noexcept -> std::string_view
{
    return nym_created_;
}

auto Endpoints::NymDownload() const noexcept -> std::string_view
{
    return nym_download_;
}

auto Endpoints::OTDHTBlockchain(
    const opentxs::blockchain::Type chain) const noexcept -> std::string_view
{
    return otdht_blockchain_.at(chain);
}

auto Endpoints::OTDHTNodePublish() const noexcept -> std::string_view
{
    return otdht_node_publish_;
}

auto Endpoints::OTDHTNodePull() const noexcept -> std::string_view
{
    return otdht_node_pull_;
}

auto Endpoints::OTDHTNodeRouter() const noexcept -> std::string_view
{
    return otdht_node_router_;
}

auto Endpoints::OTDHTWallet() const noexcept -> std::string_view
{
    return otdht_wallet_;
}

auto Endpoints::PairEvent() const noexcept -> std::string_view
{
    return pair_event_;
}

auto Endpoints::PeerReply() const noexcept -> std::string_view
{
    return peer_reply_new_;
}

auto Endpoints::PeerReplyUpdate() const noexcept -> std::string_view
{
    return peer_reply_update_;
}

auto Endpoints::PeerRequest() const noexcept -> std::string_view
{
    return peer_request_new_;
}

auto Endpoints::PeerRequestUpdate() const noexcept -> std::string_view
{
    return peer_request_update_;
}

auto Endpoints::PendingBailment() const noexcept -> std::string_view
{
    return pending_bailment_;
}

auto Endpoints::ProcessPushNotification() const noexcept -> std::string_view
{
    return process_push_notification_;
}

auto Endpoints::PushNotification() const noexcept -> std::string_view
{
    return push_notification_;
}

auto Endpoints::SeedUpdated() const noexcept -> std::string_view
{
    return seed_updated_;
}

auto Endpoints::ServerReplyReceived() const noexcept -> std::string_view
{
    return server_reply_received_;
}

auto Endpoints::ServerRequestSent() const noexcept -> std::string_view
{
    return server_request_sent_;
}

auto Endpoints::ServerUpdate() const noexcept -> std::string_view
{
    return server_update_;
}

auto Endpoints::Shutdown() const noexcept -> std::string_view
{
    return shutdown_;
}

auto Endpoints::TaskComplete() const noexcept -> std::string_view
{
    return task_complete_;
}

auto Endpoints::ThreadUpdate(const std::string_view thread) const noexcept
    -> std::string_view
{
    auto handle = thread_map_.lock();
    auto key = CString{thread};

    if (auto it = handle->find(key); handle->end() != it) {

        return it->second;
    } else {
        static constexpr auto prefix{"threadupdate"};
        auto [out, added] = handle->try_emplace(
            std::move(key), build_inproc_path(prefix, version_1_, thread));

        return out->second;
    }
}

auto Endpoints::UnitUpdate() const noexcept -> std::string_view
{
    return unit_update_;
}

auto Endpoints::WidgetUpdate() const noexcept -> std::string_view
{
    return widget_update_;
}

auto Endpoints::WorkflowAccountUpdate() const noexcept -> std::string_view
{
    return workflow_account_update_;
}
}  // namespace opentxs::api::session::imp
