// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::Language

#include "api/context/Context.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>  // IWYU pragma: keep
#include <future>
#include <iosfwd>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/Crypto.hpp"
#include "internal/api/Factory.hpp"
#include "internal/api/Settings.hpp"
#include "internal/api/crypto/Encode.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/notary/Notary.hpp"
#include "internal/core/String.hpp"
#include "internal/interface/rpc/RPC.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Log.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/Signals.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Language.hpp"   // IWYU pragma: keep
#include "opentxs/crypto/SeedStyle.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordCallback.hpp"
#include "opentxs/util/PasswordCaller.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep

namespace opentxs::factory
{
auto Context(
    const network::zeromq::Context& zmq,
    const api::network::Asio& asio,
    const internal::ShutdownSender& sender,
    const Options& args,
    Flag& running,
    std::promise<void>& shutdown,
    PasswordCaller* externalPasswordCallback) noexcept
    -> std::shared_ptr<api::internal::Context>
{
    using ReturnType = api::imp::Context;

    return std::make_shared<ReturnType>(
        args, zmq, asio, sender, running, shutdown, externalPasswordCallback);
}
}  // namespace opentxs::factory

namespace opentxs::api
{
auto Context::PrepareSignalHandling() noexcept -> void { Signals::Block(); }

auto Context::SuggestFolder(std::string_view appName) noexcept
    -> std::filesystem::path
{
    return Legacy::SuggestFolder(appName);
}
}  // namespace opentxs::api

namespace opentxs::api::internal
{
auto Context::MaxJobs() noexcept -> unsigned int
{
    return imp::Context::JobCount().load();
}

auto Context::SetMaxJobs(const opentxs::Options& args) noexcept -> void
{
    imp::Context::JobCount().store(args.MaxJobs());
}
}  // namespace opentxs::api::internal

namespace opentxs::api::imp
{
auto Context::Sessions::clear(
    const opentxs::network::zeromq::Context& zmq) noexcept -> void
{
    shutdown_ = true;
    auto futures = Vector<std::future<void>>{};
    futures.reserve(server_.size() + client_.size());

    for (auto& session : server_) {
        futures.emplace_back(session->Internal().Stop());
    }

    for (auto& session : client_) {
        futures.emplace_back(session->Internal().Stop());
    }

    server_.clear();
    client_.clear();
    auto done{true};

    for (auto& future : futures) {
        using Status = std::future_status;

        if (Status::ready != future.wait_for(30s)) { done = false; }
    }

    if (false == done) {
        LogError()(OT_PRETTY_CLASS())(
            "shutdown delayed, possibly due to active zmq batches.")
            .Flush();
        LogError()(zmq.Internal().ActiveBatches()).Flush();
    }

    for (auto& future : futures) { future.get(); }
}
}  // namespace opentxs::api::imp

namespace opentxs::api::imp
{
Context::Context(
    const opentxs::Options& args,
    const opentxs::network::zeromq::Context& zmq,
    const network::Asio& asio,
    const opentxs::internal::ShutdownSender& sender,
    Flag& running,
    std::promise<void>& shutdown,
    PasswordCaller* password)
    : api::internal::Context()
    , running_(running)
    , shutdown_(shutdown)
    , args_(args)
    , zmq_context_(zmq)
    , asio_(asio)
    , shutdown_sender_(sender)
    , home_(args_.Home().string().c_str())
    , null_callback_(opentxs::Factory::NullCallback())
    , default_external_password_callback_([&] {
        auto out = std::make_unique<PasswordCaller>();

        OT_ASSERT(out);

        out->SetCallback(null_callback_.get());

        return out;
    }())
    , external_password_callback_([&] {
        if (nullptr == password) {

            return default_external_password_callback_.get();
        } else {

            return password;
        }
    }())
    , profile_id_()
    , periodic_(std::nullopt)
    , legacy_(factory::Legacy(home_))
    , config_()
    , crypto_(nullptr)
    , factory_(nullptr)
    , zap_(nullptr)
    , sessions_()
    , rpc_(opentxs::Factory::RPC(*this))
    , file_lock_()
    , signal_handler_()
{
    OT_ASSERT(null_callback_);
    OT_ASSERT(default_external_password_callback_);
    OT_ASSERT(zmq_context_);
    OT_ASSERT(legacy_);
    OT_ASSERT(nullptr != external_password_callback_);
    OT_ASSERT(external_password_callback_->HaveCallback());
    OT_ASSERT(rpc_);
}

auto Context::Cancel(const TaskID task) const -> bool
{
    return periodic_->Cancel(task);
}

auto Context::client_instance(const int count) -> int
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count);
}

auto Context::ClientSession(const int instance) const
    -> const api::session::Client&
{
    const auto& output = sessions_.lock_shared()->client_.at(instance);

    OT_ASSERT(output);

    return *output;
}

auto Context::Config(const std::filesystem::path& path) const noexcept
    -> const api::Settings&
{
    const auto& config = [&]() -> auto&
    {
        auto handle = config_.lock();
        auto& map = *handle;

        if (auto i = map.find(path); map.end() == i) {
            const auto [out, rc] = map.try_emplace(
                path,
                factory::Settings(
                    *legacy_, String::Factory(path.string().c_str())));

            OT_ASSERT(rc);

            return out->second;
        } else {

            return i->second;
        }
    }
    ();

    OT_ASSERT(config);

    return *config;
}

auto Context::Crypto() const noexcept -> const api::Crypto&
{
    OT_ASSERT(crypto_);

    return *crypto_;
}

auto Context::Factory() const noexcept -> const api::Factory&
{
    OT_ASSERT(factory_);

    return *factory_;
}

auto Context::GetPasswordCaller() const noexcept -> PasswordCaller&
{
    OT_ASSERT(nullptr != external_password_callback_);

    return *external_password_callback_;
}

auto Context::ProfileId() const noexcept -> std::string_view
{
    return profile_id_.get();
}

auto Context::Init() noexcept -> void
{
    Init_Log();
    Init_Periodic();
    init_pid();
    Init_Crypto();
    Init_Factory();
    Init_Profile();
    Init_Rlimit();
    Init_CoreDump();
    Init_Zap();
}

auto Context::Init_Crypto() -> void
{
    crypto_ =
        factory::CryptoAPI(Config(legacy_->OpentxsConfigFilePath().string()));

    OT_ASSERT(crypto_);
}

auto Context::Init_Factory() -> void
{
    factory_ = factory::FactoryAPI(*crypto_);

    OT_ASSERT(factory_);

    crypto_->Internal().Init(factory_);
}

auto Context::Init_Log() -> void
{
    OT_ASSERT(legacy_);

    const auto& config = Config(legacy_->OpentxsConfigFilePath().string());
    auto notUsed{false};
    auto level = std::int64_t{0};
    const auto value = args_.LogLevel();

    if (-1 > value) {
        config.Internal().CheckSet_long(
            String::Factory("logging"),
            String::Factory("log_level"),
            0,
            level,
            notUsed);
    } else {
        config.Internal().Set_long(
            String::Factory("logging"),
            String::Factory("log_level"),
            value,
            notUsed);
        level = value;
    }

    opentxs::internal::Log::SetVerbosity(static_cast<int>(level));
}

auto Context::Init_Periodic() -> void
{
    periodic_.emplace(asio_);

    OT_ASSERT(periodic_.has_value());
}

auto Context::init_pid() const -> void
{
    try {
        const auto path = legacy_->PIDFilePath();
        {
            std::ofstream(path.c_str());
        }

        auto lock = boost::interprocess::file_lock{path.c_str()};

        if (false == lock.try_lock()) {
            throw std::runtime_error(
                "Another process has locked the data directory");
        }

        file_lock_.swap(lock);
    } catch (const std::exception& e) {
        LogConsole()(e.what()).Flush();

        OT_FAIL;
    }
}

auto Context::Init_Profile() -> void
{
    const auto& config = Config(legacy_->OpentxsConfigFilePath().string());
    auto profile_id_exists{false};
    auto existing_profile_id{String::Factory()};
    config.Internal().Check_str(
        String::Factory("profile"),
        String::Factory("profile_id"),
        existing_profile_id,
        profile_id_exists);

    if (profile_id_exists) {
        profile_id_.set_value(existing_profile_id->Get());
    } else {
        const auto new_profile_id(crypto_->Encode().InternalEncode().Nonce(20));
        auto new_or_update{true};
        config.Internal().Set_str(
            String::Factory("profile"),
            String::Factory("profile_id"),
            new_profile_id,
            new_or_update);
        profile_id_.set_value(new_profile_id->Get());
    }
}

auto Context::Init_Zap() -> void
{
    zap_.reset(opentxs::Factory::ZAP(zmq_context_));

    OT_ASSERT(zap_);
}

auto Context::JobCount() noexcept -> std::atomic<unsigned int>&
{
    static auto count =
        std::atomic<unsigned int>{std::numeric_limits<unsigned int>::max()};

    return count;
}

auto Context::NotarySession(const int instance) const -> const session::Notary&
{
    const auto& output = sessions_.lock_shared()->server_.at(instance);

    OT_ASSERT(output);

    return *output;
}

auto Context::RPC(const rpc::request::Base& command) const noexcept
    -> std::unique_ptr<rpc::response::Base>
{
    return rpc_->Process(command);
}

auto Context::Reschedule(
    const TaskID task,
    const std::chrono::seconds& interval) const -> bool
{
    return periodic_->Reschedule(task, interval);
}

auto Context::Schedule(
    const std::chrono::seconds& interval,
    const PeriodicTask& task,
    const std::chrono::seconds& last) const -> TaskID
{
    return periodic_->Schedule(interval, task, last);
}

auto Context::server_instance(const int count) -> int
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count) + 1;
}

auto Context::Shutdown() noexcept -> void
{
    running_.Off();
    periodic_->Shutdown();
    signal_handler_.modify([&](auto& data) {
        if (nullptr != data.callback_) {
            auto& callback = *data.callback_;
            callback();
            data.callback_ = nullptr;
        }
    });
    rpc_.reset();
    sessions_.lock()->clear(zmq_context_);
    zap_.reset();
    shutdown_qt();
    crypto_.reset();
    legacy_.reset();
    factory_.reset();
    config_.lock()->clear();
}

auto Context::ShuttingDown() const noexcept -> bool
{
    return shutdown_sender_.Activated();
}

auto Context::StartClientSession(
    const opentxs::Options& args,
    const int instance) const -> const api::session::Client&
{
    auto handle = sessions_.lock();

    OT_ASSERT(false == handle->shutdown_);

    auto& vector = handle->client_;
    const auto existing = vector.size();
    const auto count = std::max<std::size_t>(0_uz, instance);
    const auto effective = std::min(count, existing);

    if (effective == existing) {
        const auto count = vector.size();

        OT_ASSERT(std::numeric_limits<int>::max() > count);

        const auto next = static_cast<int>(count);
        const auto instance = client_instance(next);
        auto& client = vector.emplace_back(factory::ClientSession(
            *this,
            running_,
            args_ + args,
            Config(legacy_->ClientConfigFilePath(next).string()),
            *crypto_,
            zmq_context_,
            legacy_->ClientDataFolder(next),
            instance));

        OT_ASSERT(client);

        client->InternalClient().Init();
        client->InternalClient().Start(client);

        return *client;
    } else {
        const auto& output = vector.at(effective);

        OT_ASSERT(output);

        return *output;
    }
}

auto Context::StartClientSession(const int instance) const
    -> const api::session::Client&
{
    static const auto blank = opentxs::Options{};

    return StartClientSession(blank, instance);
}

auto Context::StartClientSession(
    const opentxs::Options& args,
    const int instance,
    std::string_view recoverWords,
    std::string_view recoverPassphrase) const -> const api::session::Client&
{
    OT_ASSERT(crypto::HaveHDKeys());

    const auto& client = StartClientSession(args, instance);
    auto reason = client.Factory().PasswordPrompt("Recovering a BIP-39 seed");

    if (0 < recoverWords.size()) {
        auto wordList =
            opentxs::Context().Factory().SecretFromText(recoverWords);
        auto phrase =
            opentxs::Context().Factory().SecretFromText(recoverPassphrase);
        client.Crypto().Seed().ImportSeed(
            wordList,
            phrase,
            opentxs::crypto::SeedStyle::BIP39,
            opentxs::crypto::Language::en,
            reason);
    }

    return client;
}

auto Context::StartNotarySession(
    const opentxs::Options& args,
    const int instance) const -> const session::Notary&
{
    auto handle = sessions_.lock();

    OT_ASSERT(false == handle->shutdown_);

    auto& vector = handle->server_;
    const auto existing = vector.size();
    const auto count = std::max<std::size_t>(0_uz, instance);
    const auto effective = std::min(count, existing);

    if (effective == existing) {
        const auto count = vector.size();

        OT_ASSERT(std::numeric_limits<int>::max() > count);

        const auto next = static_cast<int>(count);
        const auto instance = server_instance(next);
        auto& server = vector.emplace_back(factory::NotarySession(
            *this,
            running_,
            args_ + args,
            *crypto_,
            Config(legacy_->ServerConfigFilePath(next).string()),
            zmq_context_,
            legacy_->ServerDataFolder(next),
            instance));

        OT_ASSERT(server);

        server->InternalNotary().Start(server);

        return *server;
    } else {
        const auto& output = vector.at(effective);

        OT_ASSERT(output);

        return *output;
    }
}

auto Context::StartNotarySession(const int instance) const
    -> const session::Notary&
{
    static const auto blank = opentxs::Options{};

    return StartNotarySession(blank, instance);
}

auto Context::ZAP() const noexcept -> const api::network::ZAP&
{
    OT_ASSERT(zap_);

    return *zap_;
}

auto Context::ZMQ() const noexcept -> const opentxs::network::zeromq::Context&
{
    return zmq_context_;
}

Context::~Context() { shutdown_.set_value(); }
}  // namespace opentxs::api::imp
