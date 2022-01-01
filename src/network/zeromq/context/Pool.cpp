// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "network/zeromq/context/Pool.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <thread>

#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/context/Thread.hpp"
#include "opentxs/Types.hpp"

namespace opentxs::network::zeromq::context
{
Pool::Pool(const Context& parent) noexcept
    : parent_(parent)
    , count_(std::thread::hardware_concurrency())
    , index_lock_()
    , batch_lock_()
    , gate_()
    , threads_()
    , batches_()
    , batch_index_()
    , socket_index_()
{
    for (unsigned int n{0}; n < count_; ++n) { threads_.try_emplace(n, *this); }
}

auto Pool::DoModify(SocketID id, ModifyCallback& cb) noexcept -> bool
{
    try {
        auto lock = Lock{index_lock_};
        auto [batch, socket] = socket_index_.at(id);
        cb(*socket);

        return true;
    } catch (...) {

        return false;
    }
}

auto Pool::get(BatchID id) noexcept -> Thread&
{
    return threads_.at(id % count_);
}

auto Pool::MakeBatch(std::pmr::vector<socket::Type>&& types) noexcept
    -> internal::Batch&
{
    auto id = GetBatchID();
    auto lock = Lock{batch_lock_};
    auto [it, added] = batches_.try_emplace(id, id, parent_, std::move(types));

    assert(added);

    return it->second;
}

auto Pool::Modify(SocketID id, ModifyCallback cb) noexcept -> AsyncResult
{
    const auto ticket = gate_.get();

    if (ticket) { return {}; }

    try {
        auto [batch, socket] = [&] {
            auto lock = Lock{index_lock_};

            return socket_index_.at(id);
        }();

        return get(batch).Modify(id, std::move(cb));
    } catch (...) {

        return {};
    }
}

auto Pool::Shutdown() noexcept -> void { gate_.shutdown(); }

auto Pool::Start(BatchID id, StartArgs&& sockets) noexcept
    -> zeromq::internal::Thread*
{
    const auto ticket = gate_.get();

    if (ticket) { return nullptr; }

    try {
        if (auto lock = Lock{index_lock_}; 0u < batch_index_.count(id)) {
            throw std::runtime_error{"batch already exists"};
        }

        auto& thread = get(id);

        if (thread.Add(id, std::move(sockets))) {

            return &thread;
        } else {
            throw std::runtime_error{"failed to add batch to thread"};
        }
    } catch (const std::exception& e) {
        std::cerr << OT_PRETTY_CLASS() << e.what() << std::endl;

        return nullptr;
    }
}

auto Pool::Stop(BatchID id) noexcept -> std::future<bool>
{
    try {
        auto sockets = [&] {
            auto out = std::pmr::vector<socket::Raw*>{};
            auto lock = Lock{index_lock_};

            for (const auto& sID : batch_index_.at(id)) {
                out.emplace_back(socket_index_.at(sID).second);
            }

            return out;
        }();
        auto& thread = get(id);

        return thread.Remove(id, std::move(sockets));
    } catch (...) {
        auto promise = std::promise<bool>{};
        auto output = promise.get_future();
        promise.set_value(false);

        return output;
    }
}

auto Pool::UpdateIndex(BatchID id, StartArgs&& sockets) noexcept -> void
{
    auto lock = Lock{index_lock_};
    auto& batch = batch_index_[id];

    for (auto& [sID, socket, cb] : sockets) {
        batch.emplace_back(sID);

        assert(0u == socket_index_.count(sID));

        const auto [it, added] =
            socket_index_.try_emplace(sID, std::make_pair(id, socket));

        assert(added);
    }
}

auto Pool::UpdateIndex(BatchID id) noexcept -> void
{
    {
        auto lock = Lock{index_lock_};

        if (auto batch = batch_index_.find(id); batch_index_.end() != batch) {
            for (const auto& socketID : batch->second) {
                socket_index_.erase(socketID);
            }

            batch_index_.erase(batch);
        }
    }
    {
        auto lock = Lock{batch_lock_};
        batches_.erase(id);
    }
}

Pool::~Pool()
{
    gate_.shutdown();

    for (auto& [id, thread] : threads_) { thread.Shutdown(); }
}
}  // namespace opentxs::network::zeromq::context
