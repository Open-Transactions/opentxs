// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef libopentxs_EXPORTS
#ifndef OPENTXS_STATIC_DEFINE
#error libopentxs_EXPORTS or OPENTXS_STATIC_DEFINE must be defined when building the library
#endif
#endif

#include "opentxs/OT.hpp"  // IWYU pragma: associated

#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

#include "TBB.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/Factory.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/crypto/library/OpenSSL.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Factory.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Signals.hpp"
#include "internal/util/Thread.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs
{
class Instance
{
public:
    class Monitor
    {
    public:
        auto Start() noexcept -> void
        {
            auto lock = Lock{lock_};
            running_ = true;
        }
        auto Stop() noexcept -> void
        {
            auto lock = Lock{lock_};
            running_ = false;
            cv_.notify_one();
        }
        auto Wait() noexcept -> void
        {
            auto lock = Lock{lock_};
            cv_.wait(lock, [this] { return false == running_; });
        }

    private:
        mutable std::mutex lock_{};
        bool running_{};
        std::condition_variable cv_{};
    };

    Monitor zmq_monitor_;
    Monitor context_monitor_;

    static auto get() noexcept -> Instance&
    {
        static auto singleton = Instance{};

        return singleton;
    }

    auto Context() const noexcept -> std::shared_ptr<const api::Context>
    {
        return std::atomic_load(std::addressof(context_));
    }
    auto ZMQ() const noexcept -> std::shared_ptr<const network::zeromq::Context>
    {
        return std::atomic_load(std::addressof(zmq_));
    }

    auto Join() noexcept -> std::shared_future<void>
    {
        auto lock = Lock{lock_};

        return join_.Begin([this] {
            using enum Operation;
            queue_.emplace(join, nullptr, nullptr);
            cv_.notify_one();
        });
    }
    auto Start(const Options& args, PasswordCaller* cb) noexcept
        -> std::weak_ptr<const api::Context>
    {
        auto future = [&, this] {
            auto lock = Lock{lock_};

            return start_.Begin([&, this] {
                using enum Operation;
                queue_.emplace(start, std::addressof(args), cb);
                cv_.notify_one();
            });
        }();

        return future.get();
    }
    auto Stop() noexcept -> std::shared_future<void>
    {
        auto lock = Lock{lock_};

        return stop_.Begin([this] {
            using enum Operation;
            queue_.emplace(stop, nullptr, nullptr);
            cv_.notify_one();
        });
    }

    ~Instance()
    {
        {
            auto lock = Lock{lock_};
            using enum Operation;
            queue_.emplace(shutdown, nullptr, nullptr);
            cv_.notify_one();
        }

        if (thread_.joinable()) { thread_.join(); }
    }

private:
    enum class Operation {
        start,
        stop,
        join,
        shutdown,
    };

    template <typename Result>
    struct Job {
        bool running_{};
        std::promise<Result> promise_{};
        std::shared_future<Result> future_{};

        template <typename CB>
        auto Begin(CB cb) noexcept -> std::shared_future<Result>
        {
            if (false == running_) {
                running_ = true;
                promise_ = {};
                future_ = promise_.get_future();
                std::invoke(cb);
            }

            return future_;
        }
        auto Finish() noexcept -> void
        {
            if (running_) {
                running_ = false;
                promise_.set_value();
            }
        }
        template <typename VoidHack>
        auto Finish(VoidHack value) noexcept -> void
        {
            if (running_) {
                running_ = false;
                promise_.set_value(value);
            }
        }
    };
    struct Task {
        Operation type_;
        const Options* options_;
        PasswordCaller* password_;

        Task() = delete;
        Task(
            Operation type,
            const Options* options,
            PasswordCaller* password) noexcept
            : type_(std::move(type))
            , options_(std::move(options))
            , password_(std::move(password))
        {
        }
        Task(const Task&) = delete;
        Task(Task&&) = delete;
        auto operator=(const Task&) -> Task& = delete;
        auto operator=(Task&&) -> Task& = delete;

        ~Task() = default;
    };

    std::mutex lock_;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
    tbb::Options tbb_;
#pragma GCC diagnostic pop
    std::unique_ptr<api::network::Asio> asio_;
    std::shared_ptr<network::zeromq::Context> zmq_;
    std::optional<opentxs::internal::ShutdownSender> shutdown_sender_;
    std::shared_ptr<api::Context> context_;
    Job<std::weak_ptr<api::Context>> start_;
    Job<void> stop_;
    Job<void> join_;
    std::condition_variable cv_;
    std::queue<Task> queue_;
    std::thread thread_;

    Instance() noexcept
        : zmq_monitor_()
        , context_monitor_()
        , lock_()
        , tbb_()
        , asio_(nullptr)
        , zmq_(nullptr)
        , shutdown_sender_(std::nullopt)
        , context_(nullptr)
        , start_()
        , stop_()
        , join_()
        , cv_()
        , queue_()
        , thread_(&Instance::thread, this)
    {
    }

    auto process_task(Task& task) noexcept -> bool
    {
        using enum Operation;

        switch (task.type_) {
            case start: {
                start_ot(*task.options_, task.password_);

                return true;
            }
            case stop: {
                stop_ot();

                return true;
            }
            case join: {

                return true;
            }
            case shutdown: {
                stop_ot();

                return false;
            }
            default: {
                std::terminate();
            }
        }
    }
    auto start_ot(const Options& args, PasswordCaller* cb) noexcept -> void
    {
        auto context = std::atomic_load(std::addressof(context_));

        if (false == context.operator bool()) {
            crypto::OpenSSL::InitOpenSSL();
            api::internal::Context::SetMaxJobs(args);
            auto zmq = [&] {
                auto out = factory::ZMQContext(args);
                out->Internal().Init(args, out);
                std::atomic_store(std::addressof(zmq_), out);
                zmq_monitor_.Start();

                return out;
            }();
            asio_ = factory::AsioAPI(*zmq, args.TestMode());
            using api::session::internal::Endpoints;
            shutdown_sender_.emplace(
                *asio_, *zmq, Endpoints::ContextShutdown(), "global shutdown");
            context =
                factory::Context(*zmq, *asio_, *shutdown_sender_, args, cb);
            asio_->Internal().Init(context);
            context->Internal().Init(context);
            std::atomic_store(std::addressof(context_), context);
            context_monitor_.Start();
        }

        start_.Finish(context);
    }
    auto stop_ot() noexcept -> void
    {
        if (auto context = std::atomic_exchange(std::addressof(context_), {});
            context) {
            shutdown_sender_->Activate();
            context->Internal().Shutdown();
            asio_->Internal().Shutdown();
            auto zmq = std::atomic_exchange(std::addressof(zmq_), {});
            zmq->Internal().Stop();
            context.reset();
            context_monitor_.Wait();
            asio_.reset();
            zmq.reset();
            zmq_monitor_.Wait();
            shutdown_sender_.reset();
        }

        join_.Finish();
        stop_.Finish();
    }
    auto thread() noexcept -> void
    {
        Signals::Block();
        SetThisThreadsName("ot_event_loop");
        auto running = true;

        while (running) {
            auto lock = Lock{lock_};
            const auto condition = [this] { return false == queue_.empty(); };
            cv_.wait(lock, condition);

            while (std::invoke(condition)) {
                auto& task = queue_.front();
                running = process_task(task);
                queue_.pop();

                if (false == running) { break; }
            }
        }
    }
};
}  // namespace opentxs

namespace opentxs
{
auto Cleanup() noexcept -> void { Instance::get().Stop().get(); }

auto InitContext() -> const api::Context&
{
    static const auto empty = Options{};

    return InitContext(empty, nullptr);
}

auto InitContext(const Options& args) -> const api::Context&
{
    return InitContext(args, nullptr);
}

auto InitContext(PasswordCaller* cb) -> const api::Context&
{
    static const auto empty = Options{};

    return InitContext(empty, cb);
}

auto InitContext(const Options& args, PasswordCaller* externalPasswordCallback)
    -> const api::Context&
{
    auto& instance = Instance::get();
    const auto original = instance.Context();
    auto weak = instance.Start(args, externalPasswordCallback);
    auto shared = weak.lock();

    if (shared) {
        if (original.get() == shared.get()) {

            throw std::runtime_error{"Context is already initialized"};
        } else {

            return *shared;
        }
    } else {

        throw std::runtime_error{"startup interrupted"};
    }
}

auto Join() noexcept -> void { Instance::get().Join().get(); }
}  // namespace opentxs

namespace opentxs
{
auto context_has_terminated() noexcept -> void
{
    Instance::get().context_monitor_.Stop();
}

auto get_context_for_unit_tests() noexcept
    -> std::shared_ptr<const api::Context>
{
    return Instance::get().Context();
}

auto get_zeromq() noexcept -> std::weak_ptr<const network::zeromq::Context>
{
    return Instance::get().ZMQ();
}

auto zmq_has_terminated() noexcept -> void
{
    Instance::get().zmq_monitor_.Stop();
}
}  // namespace opentxs
