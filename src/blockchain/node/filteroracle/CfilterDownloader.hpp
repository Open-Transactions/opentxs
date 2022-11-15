// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"

#pragma once

#include <cstddef>
#include <deque>
#include <exception>
#include <future>
#include <memory>
#include <vector>

#include "blockchain/DownloadManager.hpp"
#include "blockchain/DownloadTask.hpp"
#include "blockchain/node/filteroracle/Shared.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace cfilter
{
class Header;
}  // namespace cfilter

namespace node
{
namespace filteroracle
{
class CfilterDownloader;
class Shared;
}  // namespace filteroracle
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::filteroracle
{
using FilterDM =
    download::Manager<CfilterDownloader, GCS, cfilter::Header, cfilter::Type>;
using FilterWorker = Worker<CfilterDownloader, api::Session>;

class CfilterDownloader : public FilterDM, public FilterWorker
{
public:
    auto NextBatch() noexcept { return allocate_batch(shared_.default_type_); }

    CfilterDownloader(std::shared_ptr<Shared> shared) noexcept;

    ~CfilterDownloader();

private:
    friend FilterDM;
    friend FilterWorker;

    std::shared_ptr<Shared> shared_p_;
    Shared& shared_;

    auto batch_ready() const noexcept -> void;
    auto batch_size(std::size_t in) const noexcept -> std::size_t;
    auto check_task(TaskType&) const noexcept -> void;
    auto trigger_state_machine() const noexcept -> void;
    auto update_tip(const Position& position, const cfilter::Header&)
        const noexcept -> void;

    auto pipeline(const network::zeromq::Message& in) noexcept -> void;
    auto process_reset(const network::zeromq::Message& in) noexcept -> void;
    auto queue_processing(DownloadedData&& data) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto UpdatePosition(const Position& pos) -> void;
};
}  // namespace opentxs::blockchain::node::filteroracle
