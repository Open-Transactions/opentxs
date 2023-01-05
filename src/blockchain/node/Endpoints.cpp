// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Endpoints.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/network/zeromq/ZeroMQ.hpp"

namespace opentxs::blockchain::node
{
Endpoints::Endpoints(allocator_type alloc) noexcept
    : block_oracle_publish_(network::zeromq::MakeArbitraryInproc(alloc))
    , block_oracle_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , block_oracle_router_(network::zeromq::MakeArbitraryInproc(alloc))
    , block_tip_publish_(network::zeromq::MakeArbitraryInproc(alloc))
    , cfilter_downloader_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , fee_oracle_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , filter_oracle_reindex_publish_(
          network::zeromq::MakeArbitraryInproc(alloc))
    , header_oracle_job_ready_(network::zeromq::MakeArbitraryInproc(alloc))
    , header_oracle_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , manager_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , new_filter_publish_(network::zeromq::MakeArbitraryInproc(alloc))
    , new_header_publish_(network::zeromq::MakeArbitraryInproc(alloc))
    , otdht_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , peer_manager_publish_(network::zeromq::MakeArbitraryInproc(alloc))
    , peer_manager_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , peer_manager_push_(network::zeromq::MakeArbitraryInproc(alloc))
    , shutdown_publish_(network::zeromq::MakeArbitraryInproc(alloc))
    , wallet_pull_(network::zeromq::MakeArbitraryInproc(alloc))
    , wallet_to_accounts_push_(network::zeromq::MakeArbitraryInproc(alloc))
{
}

Endpoints::Endpoints(Endpoints&& rhs, allocator_type alloc) noexcept
    : block_oracle_publish_(std::move(rhs.block_oracle_publish_), alloc)
    , block_oracle_pull_(std::move(rhs.block_oracle_pull_), alloc)
    , block_oracle_router_(std::move(rhs.block_oracle_router_), alloc)
    , block_tip_publish_(std::move(rhs.block_tip_publish_), alloc)
    , cfilter_downloader_pull_(std::move(rhs.cfilter_downloader_pull_), alloc)
    , fee_oracle_pull_(std::move(rhs.fee_oracle_pull_), alloc)
    , filter_oracle_reindex_publish_(
          std::move(rhs.filter_oracle_reindex_publish_),
          alloc)
    , header_oracle_job_ready_(std::move(rhs.header_oracle_job_ready_), alloc)
    , header_oracle_pull_(std::move(rhs.header_oracle_pull_), alloc)
    , manager_pull_(std::move(rhs.manager_pull_), alloc)
    , new_filter_publish_(std::move(rhs.new_filter_publish_), alloc)
    , new_header_publish_(std::move(rhs.new_header_publish_), alloc)
    , otdht_pull_(std::move(rhs.otdht_pull_), alloc)
    , peer_manager_publish_(std::move(rhs.peer_manager_publish_), alloc)
    , peer_manager_pull_(std::move(rhs.peer_manager_pull_), alloc)
    , peer_manager_push_(std::move(rhs.peer_manager_push_), alloc)
    , shutdown_publish_(std::move(rhs.shutdown_publish_), alloc)
    , wallet_pull_(std::move(rhs.wallet_pull_), alloc)
    , wallet_to_accounts_push_(std::move(rhs.wallet_to_accounts_push_), alloc)
{
}

Endpoints::Endpoints(Endpoints&& rhs) noexcept
    : Endpoints(std::move(rhs), rhs.get_allocator())
{
}
}  // namespace opentxs::blockchain::node
