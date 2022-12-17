// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ankerl/unordered_dense.h>
#include <cs_plain_guarded.h>
#include <string_view>

#include "internal/api/session/Endpoints.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Endpoints;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Endpoints final : public internal::Endpoints
{
public:
    auto AccountUpdate() const noexcept -> std::string_view final;
    auto BlockchainAccountCreated() const noexcept -> std::string_view final;
    auto BlockchainBalance() const noexcept -> std::string_view final;
    auto BlockchainBlockAvailable() const noexcept -> std::string_view final;
    auto BlockchainBlockDownloadQueue() const noexcept
        -> std::string_view final;
    auto BlockchainBlockOracleProgress() const noexcept
        -> std::string_view final;
    auto BlockchainMempool() const noexcept -> std::string_view final;
    auto BlockchainMessageRouter() const noexcept -> std::string_view final;
    auto BlockchainNewFilter() const noexcept -> std::string_view final;
    auto BlockchainPeer() const noexcept -> std::string_view final;
    auto BlockchainPeerConnection() const noexcept -> std::string_view final;
    auto BlockchainReorg() const noexcept -> std::string_view final;
    auto BlockchainReportStatus() const noexcept -> std::string_view final;
    auto BlockchainScanProgress() const noexcept -> std::string_view final;
    auto BlockchainStartupPublish() const noexcept -> std::string_view final;
    auto BlockchainStartupPull() const noexcept -> std::string_view final;
    auto BlockchainStateChange() const noexcept -> std::string_view final;
    auto BlockchainSyncChecksumFailure() const noexcept
        -> std::string_view final;
    auto BlockchainSyncProgress() const noexcept -> std::string_view final;
    auto BlockchainSyncServerProgress() const noexcept
        -> std::string_view final;
    auto BlockchainSyncServerUpdated() const noexcept -> std::string_view final;
    auto BlockchainTransactions() const noexcept -> std::string_view final;
    auto BlockchainTransactions(const identifier::Nym& nym) const noexcept
        -> std::string_view final;
    auto BlockchainWalletUpdated() const noexcept -> std::string_view final;
    auto ConnectionStatus() const noexcept -> std::string_view final;
    auto ContactUpdate() const noexcept -> std::string_view final;
    auto FindNym() const noexcept -> std::string_view final;
    auto FindServer() const noexcept -> std::string_view final;
    auto FindUnitDefinition() const noexcept -> std::string_view final;
    auto IssuerUpdate() const noexcept -> std::string_view final;
    auto Messagability() const noexcept -> std::string_view final;
    auto MessageLoaded() const noexcept -> std::string_view final;
    auto NymCreated() const noexcept -> std::string_view final;
    auto NymDownload() const noexcept -> std::string_view final;
    auto OTDHTBlockchain(opentxs::blockchain::Type chain) const noexcept
        -> std::string_view final;
    auto OTDHTNodePublish() const noexcept -> std::string_view final;
    auto OTDHTNodePull() const noexcept -> std::string_view final;
    auto OTDHTNodeRouter() const noexcept -> std::string_view final;
    auto OTDHTWallet() const noexcept -> std::string_view final;
    auto PairEvent() const noexcept -> std::string_view final;
    auto PeerReplyUpdate() const noexcept -> std::string_view final;
    auto PeerRequestUpdate() const noexcept -> std::string_view final;
    auto PendingBailment() const noexcept -> std::string_view final;
    auto ProcessPushNotification() const noexcept -> std::string_view final;
    auto PushNotification() const noexcept -> std::string_view final;
    auto SeedUpdated() const noexcept -> std::string_view final;
    auto ServerReplyReceived() const noexcept -> std::string_view final;
    auto ServerRequestSent() const noexcept -> std::string_view final;
    auto ServerUpdate() const noexcept -> std::string_view final;
    auto Shutdown() const noexcept -> std::string_view final;
    auto TaskComplete() const noexcept -> std::string_view final;
    auto ThreadUpdate(const std::string_view thread) const noexcept
        -> std::string_view final;
    auto UnitUpdate() const noexcept -> std::string_view final;
    auto WidgetUpdate() const noexcept -> std::string_view final;
    auto WorkflowAccountUpdate() const noexcept -> std::string_view final;

    Endpoints(const int instance) noexcept;
    Endpoints() = delete;
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    auto operator=(const Endpoints&) -> Endpoints& = delete;
    auto operator=(Endpoints&&) -> Endpoints& = delete;

    ~Endpoints() final = default;

private:
    static constexpr auto version_1_{1};

    using BlockchainMap =
        ankerl::unordered_dense::map<opentxs::blockchain::Type, CString>;
    using BlockchainTransactionsMap =
        ankerl::unordered_dense::map<identifier::Nym, CString>;
    using ThreadMap = Map<CString, CString>;
    const int instance_;
    const CString account_update_;
    const CString blockchain_account_created_;
    const CString blockchain_balance_;
    const CString blockchain_block_available_;
    const CString blockchain_block_download_queue_;
    const CString blockchain_mempool_;
    const CString blockchain_message_router_;
    const CString blockchain_oracle_progress_;
    const CString blockchain_new_filter_;
    const CString blockchain_peer_;
    const CString blockchain_peer_connection_;
    const CString blockchain_reorg_;
    const CString blockchain_report_status_;
    const CString blockchain_scan_progress_;
    const CString blockchain_startup_publish_;
    const CString blockchain_startup_pull_;
    const CString blockchain_state_change_;
    const CString blockchain_sync_checksum_failure_;
    const CString blockchain_sync_progress_;
    const CString blockchain_sync_server_progress_;
    const CString blockchain_server_updated_;
    const CString blockchain_transactions_;
    const CString blockchain_wallet_updated_;
    const CString connection_status_;
    const CString contact_update_;
    const CString find_nym_;
    const CString find_server_;
    const CString find_unit_definition_;
    const CString issuer_update_;
    const CString messagability_;
    const CString message_loaded_;
    const CString nym_created_;
    const CString nym_download_;
    const BlockchainMap otdht_blockchain_;
    const CString otdht_node_router_;
    const CString otdht_node_publish_;
    const CString otdht_node_pull_;
    const CString otdht_wallet_;
    const CString pair_event_;
    const CString peer_reply_update_;
    const CString peer_request_update_;
    const CString pending_bailment_;
    const CString process_push_notification_;
    const CString push_notification_;
    const CString seed_updated_;
    const CString server_reply_received_;
    const CString server_request_sent_;
    const CString server_update_;
    const CString shutdown_;
    const CString task_complete_;
    const CString unit_update_;
    const CString widget_update_;
    const CString workflow_account_update_;
    mutable libguarded::plain_guarded<BlockchainTransactionsMap>
        blockchain_transactions_map_;
    mutable libguarded::plain_guarded<ThreadMap> thread_map_;

    auto build_inproc_path(const std::string_view path, const int version)
        const noexcept -> CString;
    auto build_inproc_path(
        const std::string_view path,
        const int version,
        const std::string_view suffix) const noexcept -> CString;
};
}  // namespace opentxs::api::session::imp
