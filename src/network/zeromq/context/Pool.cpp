// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/context/Pool.hpp"  // IWYU pragma: associated

#include <zmq.h>  // IWYU pragma: keep
#include <algorithm>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <type_traits>

#include "internal/api/Legacy.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Thread.hpp"
#include "network/zeromq/context/Thread.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::network::zeromq::context
{
Pool::Pool(
    const opentxs::Options& args,
    std::shared_ptr<const Context> parent) noexcept
    : parent_p_(parent)
    , parent_(*parent_p_)
    , count_(MaxJobs())
    , log_dir_(api::Legacy::Home(args) / "alloc" / "batch")
    , write_(args.DebugAllocations())
    , shutdown_counter_()
    , running_(true)
    , gate_()
    , notify_()
    , threads_()
    , batches_()
    , index_()
    , start_args_()
    , stop_args_()
    , modify_args_()
    , allocators_()
{
    if (write_) { std::cout << "writing log files for allocator debugging\n"; }

    for (unsigned int n{0}; n < count_; ++n) {
        auto [i, rc] = notify_.try_emplace(
            n,
            std::make_pair(
                MakeArbitraryInproc(alloc::Default{}),
                parent_.Internal().RawSocket(socket::Type::Push)));

        if (false == rc) { std::terminate(); }

        auto& [endpoint, socket] = i->second;
        rc = socket.lock()->Bind(endpoint.c_str());

        if (false == rc) { std::terminate(); }

        threads_.try_emplace(n, n, *this, endpoint);
    }

    std::filesystem::remove_all(log_dir_);
    std::filesystem::create_directories(log_dir_);
}

auto Pool::ActiveBatches(alloc::Default alloc) const noexcept -> CString
{
    auto handle = batches_.lock_shared();
    const auto& map = *handle;

    if (map.empty()) {

        return {"no batches", alloc};
    } else {
        // TODO c++20 allocator
        auto out = std::stringstream{"batches:\n"};

        for (const auto& [id, batch] : map) {
            out << "ID: " << id << ", Name: " << batch->thread_name_ << '\n';
        }

        return {out.str().c_str(), alloc};
    }
}

auto Pool::Alloc(BatchID id) noexcept -> alloc::Logging*
{
    return std::addressof(allocators_.lock()->at(id));
}

auto Pool::allocate_next_batch() const noexcept -> BatchID
{
    const auto id = GetBatchID();
    const auto& thread = get(id);
    const auto filename =
        (log_dir_ / std::to_string(id)).replace_extension("csv");
    auto* upstream = thread.Alloc();
    allocators_.lock()->try_emplace(id, filename, write_, upstream);

    return id;
}

auto Pool::BelongsToThreadPool(const std::thread::id id) const noexcept -> bool
{
    auto alloc = alloc::MonotonicUnsync{1024};
    auto threads = Set<std::thread::id>{&alloc};

    for (const auto& [tid, thread] : threads_) { threads.emplace(thread.ID()); }

    return 0_uz < threads.count(id);
}

auto Pool::DoModify(SocketID id) noexcept -> void
{
    const auto ticket = gate_.get();

    if (ticket) { return; }

    auto null = factory::ZMQSocketNull();

    if (null.ID() == id) {
        auto callbackHandle = modify_args_.lock();

        try {
            auto& callbacks = callbackHandle->at(id);

            for (const auto& callback : callbacks) {
                try {
                    if (false == callback.operator bool()) { std::terminate(); }

                    callback(null);
                } catch (const std::exception& e) {
                    std::cerr << OT_PRETTY_CLASS() << e.what() << std::endl;
                }
            }

            callbacks.clear();
        } catch (...) {
        }
    } else {
        const auto indexHandle = index_.lock_shared();
        const auto& [bIndex, sIndex] = *indexHandle;
        auto callbackHandle = modify_args_.lock();

        try {
            auto* socket = sIndex.at(id).second;
            auto& callbacks = callbackHandle->at(id);

            for (const auto& callback : callbacks) {
                try {
                    if (false == callback.operator bool()) { std::terminate(); }

                    callback(*socket);
                } catch (const std::exception& e) {
                    std::cerr << OT_PRETTY_CLASS() << e.what() << std::endl;
                }
            }

            callbacks.clear();
        } catch (...) {
        }
    }
}

auto Pool::GetStartArgs(BatchID id) noexcept -> ThreadStartArgs
{
    auto args = [&] {
        auto handle = start_args_.lock();
        auto& map = *handle;
        auto i = map.find(id);

        if (map.end() == i) { std::terminate(); }

        auto post = ScopeGuard{[&] { map.erase(i); }};

        return std::move(i->second);
    }();
    auto sockets = [&] {
        auto out = ThreadStartArgs{};
        std::ranges::transform(args, std::back_inserter(out), [](auto& val) {
            auto& [sID, socket, cb] = val;

            return std::make_pair(socket, std::move(cb));
        });

        return out;
    }();

    return sockets;
}

auto Pool::GetStopArgs(BatchID id) noexcept -> Set<void*>
{
    auto args = [&] {
        auto handle = stop_args_.lock();
        auto& map = *handle;
        auto i = map.find(id);

        if (map.end() == i) { std::terminate(); }

        auto post = ScopeGuard{[&] { map.erase(i); }};

        return std::move(i->second);
    }();
    stop_batch(id);

    return args;
}

auto Pool::get(BatchID id) const noexcept -> const context::Thread&
{
    return threads_.at(id % count_);
}

auto Pool::get(BatchID id) noexcept -> context::Thread&
{
    return threads_.at(id % count_);
}

auto Pool::MakeBatch(
    Vector<socket::Type>&& types,
    std::string_view name) noexcept -> internal::Handle
{
    return MakeBatch(allocate_next_batch(), std::move(types), name);
}

auto Pool::MakeBatch(
    const BatchID id,
    Vector<socket::Type>&& types,
    std::string_view name) noexcept -> internal::Handle
{
    auto pBatch = std::shared_ptr<internal::Batch>{};
    batches_.modify([&](auto& batches) {
        const auto [it, added] = batches.try_emplace(
            id,
            std::make_shared<internal::Batch>(
                id, parent_, std::move(types), name));

        if (false == added) { std::terminate(); }

        pBatch = it->second;
    });

    if (false == pBatch.operator bool()) { std::terminate(); }

    auto& batch = *pBatch;
    index_.modify([&](auto& index) {
        auto& [bIndex, sIndex] = index;
        auto& sockets = bIndex[batch.id_];

        if (false == sockets.empty()) { std::terminate(); }

        sockets.reserve(batch.sockets_.size());

        for (auto& socket : batch.sockets_) {
            const auto sID = socket.ID();
            sockets.emplace_back(sID);

            if (0_uz != sIndex.count(sID)) { std::terminate(); }

            const auto [i, rc] = sIndex.try_emplace(
                sID, std::make_pair(batch.id_, std::addressof(socket)));

            if (false == rc) { std::terminate(); }
        }
    });

    return {parent_p_, pBatch};
}

auto Pool::Modify(SocketID id, ModifyCallback cb) noexcept -> void
{
    const auto ticket = gate_.get();

    if (ticket) { return; }

    try {
        const auto batchID = index_.lock_shared()->socket_.at(id).first;
        {
            auto handle = modify_args_.lock();
            auto& map = *handle;
            map[id].emplace_back(std::move(cb));
        }
        auto& notify = socket(batchID);
        const auto rc = notify.lock()->Send(
            [&] {
                auto out = MakeWork(Operation::change_socket);
                out.AddFrame(id);

                return out;
            }(),
            __FILE__,
            __LINE__);

        if (false == rc) {

            throw std::runtime_error{"failed to queue socket modification"};
        }
    } catch (const std::exception& e) {
        std::cerr << OT_PRETTY_CLASS() << e.what() << std::endl;
    }
}

auto Pool::PreallocateBatch() const noexcept -> BatchID
{
    return allocate_next_batch();
}

auto Pool::ReportShutdown(unsigned int index) noexcept -> void
{
    notify_.at(index).second.lock()->Close();

    if (++shutdown_counter_ == count_) { stop(); }
}

auto Pool::Shutdown() noexcept -> void
{
    if (auto running = running_.exchange(false); running) {
        gate_.shutdown();

        for (auto& [id, data] : notify_) {
            auto& socket = data.second;
            socket.lock()->Send(
                MakeWork(Operation::shutdown), __FILE__, __LINE__);
        }
    }
}

auto Pool::socket(BatchID id) noexcept -> GuardedSocket&
{
    return notify_.at(id % count_).second;
}

auto Pool::Start(BatchID id, StartArgs&& sockets) noexcept
    -> zeromq::internal::Thread*
{
    const auto ticket = gate_.get();

    if (ticket) { return nullptr; }

    try {
        {
            auto handle = start_args_.lock();
            auto& map = *handle;
            auto [i, rc] = map.try_emplace(id, std::move(sockets));

            if (false == rc) {
                throw std::runtime_error{"batch already added"};
            }
        }

        auto& thread = get(id);
        auto& notify = socket(id);
        const auto rc = notify.lock()->Send(
            [&] {
                auto out = MakeWork(Operation::add_socket);
                out.AddFrame(id);

                return out;
            }(),
            __FILE__,
            __LINE__);

        if (rc) {

            return std::addressof(thread);
        } else {
            throw std::runtime_error{"failed to add batch to thread"};
        }
    } catch (const std::exception& e) {
        std::cerr << OT_PRETTY_CLASS() << e.what() << std::endl;

        return nullptr;
    }
}

auto Pool::Stop(BatchID id) noexcept -> void
{
    if (const auto ticket = gate_.get(); ticket) { return; }

    try {
        {
            auto sockets = [&] {
                auto out = Set<void*>{};
                auto handle = index_.lock_shared();
                const auto& [bIndex, sIndex] = *handle;

                for (const auto& sID : bIndex.at(id)) {
                    out.emplace(sIndex.at(sID).second->Native());
                }

                return out;
            }();
            auto handle = stop_args_.lock();
            auto& map = *handle;
            const auto [i, rc] = map.try_emplace(id, std::move(sockets));

            if (false == rc) {

                throw std::runtime_error{"failed queue socket list"};
            }
        }

        auto& notify = socket(id);
        const auto rc = notify.lock()->Send(
            [&] {
                auto out = MakeWork(Operation::remove_socket);
                out.AddFrame(id);

                return out;
            }(),
            __FILE__,
            __LINE__);

        if (false == rc) { throw std::runtime_error{"failed stop batch"}; }
    } catch (const std::exception& e) {
        std::cerr << OT_PRETTY_CLASS() << e.what() << std::endl;
    }
}

auto Pool::stop() noexcept -> void
{
    batches_.modify([](auto& map) { map.clear(); });
    index_.modify([](auto& index) { index.clear(); });
    start_args_.lock()->clear();
    stop_args_.lock()->clear();
    modify_args_.lock()->clear();
    threads_.clear();
    notify_.clear();
    parent_p_.reset();
}

auto Pool::stop_batch(BatchID id) noexcept -> void
{
    auto deletedSockets = Set<SocketID>{};
    index_.modify([&](auto& index) {
        auto& [bIndex, sIndex] = index;

        if (auto batch = bIndex.find(id); bIndex.end() != batch) {
            for (const auto& socketID : batch->second) {
                sIndex.erase(socketID);
                deletedSockets.emplace(socketID);
            }

            bIndex.erase(batch);
        }
    });
    {
        auto handle = modify_args_.lock();
        auto& map = *handle;

        for (const auto& socketID : deletedSockets) { map.erase(socketID); }
    }
    batches_.modify([&](auto& batch) { batch.erase(id); });
    allocators_.lock()->at(id).close();
}

auto Pool::Thread(BatchID id) const noexcept -> zeromq::internal::Thread*
{
    auto& thread = static_cast<zeromq::internal::Thread&>(
        const_cast<Pool*>(this)->get(id));

    return &thread;
}

auto Pool::ThreadID(BatchID id) const noexcept -> std::thread::id
{
    return get(id).ID();
}

Pool::~Pool() = default;
}  // namespace opentxs::network::zeromq::context
