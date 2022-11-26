// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "opentxs/OT.hpp"  // IWYU pragma: associated

#include <cassert>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

#include "core/Shutdown.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/Factory.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Factory.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs
{
class Instance final
{
public:
    using SharedInstance = std::shared_ptr<Instance>;
    using SharedZMQ = std::shared_ptr<network::zeromq::Context>;

    static auto get() noexcept -> SharedInstance*
    {
        static auto instance = SharedInstance{};

        return std::addressof(instance);
    }
    static auto get_zmq() noexcept -> SharedZMQ*
    {
        static auto instance = SharedZMQ{nullptr};

        return std::addressof(instance);
    }

    auto Context() const -> const api::Context&
    {
        assert(context_);

        return *context_;
    }
    auto Join() const noexcept -> void { shutdown_.get(); }

    auto Cleanup() noexcept -> void
    {
        auto lock = Lock{lock_};

        if (context_) {
            {
                shutdown_sender_->Activate();
                context_->Shutdown();
                context_.reset();
                Join();
                shutdown_sender_.reset();
            }
            {
                asio_->Internal().Shutdown();
                asio_.reset();
            }
            {
                static auto* zmq = get_zmq();
                auto p = std::atomic_load(zmq);
                assert(p);
                auto future = p->Internal().Stop();
                p.reset();
                std::atomic_store(zmq, p);
                future.get();
            }
        } else {
            shutdown();
        }
    }
    auto Init(const Options& args, PasswordCaller* externalPasswordCallback)
        -> const api::Context&
    {
        auto lock = Lock{lock_};

        if (context_) {
            throw std::runtime_error("Context is already initialized");
        }

        init();
        api::internal::Context::SetMaxJobs(args);
        auto zmq = [&] {
            auto out = factory::ZMQContext(args);
            out->Internal().Init(out);
            std::atomic_store(get_zmq(), out);

            return out;
        }();
        {
            asio_ = factory::AsioAPI(*zmq);
        }
        {
            using Endpoints = api::session::internal::Endpoints;
            shutdown_sender_.emplace(
                *asio_, *zmq, Endpoints::ContextShutdown(), "global shutdown");
            context_ = factory::Context(
                *zmq,
                *asio_,
                *shutdown_sender_,
                args,
                running_,
                shutdown_promise_,
                externalPasswordCallback);
            asio_->Internal().Init(context_);
            context_->Init();
        }

        return *context_;
    }

    Instance() noexcept
        : lock_()
        , shutdown_promise_()
        , shutdown_(shutdown_promise_.get_future())
        , running_(Flag::Factory(true))
        , asio_(nullptr)
        , context_(nullptr)
        , shutdown_sender_(std::nullopt)
    {
    }
    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    auto operator=(const Instance&) -> Instance& = delete;
    auto operator=(Instance&&) -> Instance& = delete;

    ~Instance() { Join(); }

private:
    std::mutex lock_;
    std::promise<void> shutdown_promise_;
    std::shared_future<void> shutdown_;
    OTFlag running_;
    std::unique_ptr<api::network::Asio> asio_;
    std::shared_ptr<api::internal::Context> context_;
    std::optional<opentxs::internal::ShutdownSender> shutdown_sender_;

    auto init() noexcept -> void
    {
        shutdown();
        shutdown_promise_ = {};
        shutdown_ = shutdown_promise_.get_future();
    }
    auto shutdown() noexcept -> void
    {
        try {
            shutdown_promise_.set_value();
        } catch (...) {
        }
    }
};

auto Context() -> const api::Context&
{
    static auto* instance = Instance::get();
    auto p = std::atomic_load(instance);

    if (p) {

        return p->Context();
    } else {
        const auto error =
            CString{"Context is not initialized\n"}.append(PrintStackTrace());

        throw std::runtime_error(error.c_str());
    }
}

auto Cleanup() noexcept -> void
{
    static auto* instance = Instance::get();
    auto p = std::atomic_load(instance);

    if (p) {
        p->Cleanup();
        p.reset();
        std::atomic_store(instance, p);
    }
}

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
    static auto* instance = Instance::get();
    auto p = std::atomic_load(instance);

    if (false == p.operator bool()) {
        p = std::make_shared<Instance>();
        std::atomic_store(instance, p);
    }

    return p->Init(args, externalPasswordCallback);
}

auto Join() noexcept -> void
{
    static auto* instance = Instance::get();
    auto p = std::atomic_load(instance);

    if (p) {
        p->Join();
        p.reset();
        std::atomic_store(instance, p);
    }
}
}  // namespace opentxs

namespace opentxs
{
auto get_zeromq() noexcept -> std::weak_ptr<const network::zeromq::Context>
{
    static auto* zmq = Instance::get_zmq();

    return std::atomic_load(zmq);
}
}  // namespace opentxs
