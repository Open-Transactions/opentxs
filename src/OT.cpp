// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "opentxs/OT.hpp"  // IWYU pragma: associated

#include <cassert>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>

#include "core/Shutdown.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/Factory.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Factory.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs
{
class Instance final
{
public:
    static auto get() noexcept -> Instance&
    {
        static auto instance = Instance{};

        return instance;
    }

    auto Context() const -> const api::Context&
    {
        if (false == context_.operator bool()) {
            const auto error = CString{"Context is not initialized\n"}.append(
                PrintStackTrace());

            throw std::runtime_error(error.c_str());
        }

        return *context_;
    }
    auto Join() const noexcept -> void { shutdown_.get(); }
    auto ZMQ() const noexcept -> const network::zeromq::Context&
    {
        assert(zmq_);

        return *zmq_;
    }

    auto Cleanup() noexcept -> void
    {
        if (context_) {
            shutdown_sender_->Activate();
            context_->Shutdown();
            context_.reset();
            Join();
            shutdown_sender_.reset();
            asio_->Internal().Shutdown();
            asio_.reset();
            auto zmq = zmq_->Internal().Stop();
            zmq_.reset();
            zmq.get();
        } else {
            shutdown();
        }
    }
    auto Init(const Options& args, PasswordCaller* externalPasswordCallback)
        -> const api::Context&
    {
        if (context_) {
            throw std::runtime_error("Context is already initialized");
        }

        init();
        api::internal::Context::SetMaxJobs(args);
        zmq_ = [&] {
            auto zmq = factory::ZMQContext(args);
            zmq->Internal().Init(zmq);

            return zmq;
        }();
        asio_ = factory::AsioAPI(*zmq_);
        using Endpoints = api::session::internal::Endpoints;
        shutdown_sender_.emplace(
            *asio_, *zmq_, Endpoints::ContextShutdown(), "global shutdown");
        context_ = factory::Context(
            *zmq_,
            *asio_,
            *shutdown_sender_,
            args,
            running_,
            shutdown_promise_,
            externalPasswordCallback);
        asio_->Internal().Init(context_);
        context_->Init();

        return *context_;
    }

    Instance(const Instance&) = delete;
    Instance(Instance&&) = delete;
    auto operator=(const Instance&) -> Instance& = delete;
    auto operator=(Instance&&) -> Instance& = delete;

    ~Instance() { Join(); }

private:
    std::promise<void> shutdown_promise_;
    std::shared_future<void> shutdown_;
    OTFlag running_;
    std::shared_ptr<network::zeromq::Context> zmq_;
    std::unique_ptr<api::network::Asio> asio_;
    std::shared_ptr<api::internal::Context> context_;
    std::optional<opentxs::internal::ShutdownSender> shutdown_sender_;

    Instance() noexcept
        : shutdown_promise_()
        , shutdown_(shutdown_promise_.get_future())
        , running_(Flag::Factory(true))
        , zmq_(nullptr)
        , asio_(nullptr)
        , context_(nullptr)
        , shutdown_sender_(std::nullopt)
    {
    }

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

auto Context() -> const api::Context& { return Instance::get().Context(); }

auto Cleanup() noexcept -> void { Instance::get().Cleanup(); }

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
    return Instance::get().Init(args, externalPasswordCallback);
}

auto Join() noexcept -> void { Instance::get().Join(); }
}  // namespace opentxs

namespace opentxs
{
auto get_zeromq() noexcept -> const opentxs::network::zeromq::Context&
{
    return Instance::get().ZMQ();
}
}  // namespace opentxs
