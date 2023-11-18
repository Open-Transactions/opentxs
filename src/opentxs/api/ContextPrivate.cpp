// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/ContextPrivate.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>  // IWYU pragma: keep
#include <functional>
#include <future>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/Crypto.hpp"
#include "internal/api/crypto/Encode.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/interface/rpc/RPC.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Log.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Settings.internal.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/internal.factory.hpp"
#include "opentxs/api/session/Client.internal.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.internal.hpp"
#include "opentxs/api/session/internal.factory.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Language.hpp"         // IWYU pragma: keep
#include "opentxs/crypto/SeedStyle.hpp"        // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"  // IWYU pragma: keep
#include "opentxs/internal.factory.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordCallback.hpp"
#include "opentxs/util/PasswordCaller.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep
#include "util/Shutdown.hpp"

namespace opentxs::api
{
auto ContextPrivate::Sessions::clear(
    const opentxs::network::zeromq::Context& zmq) noexcept -> void
{
    shutdown_ = true;
    auto futures = Vector<std::future<void>>{};
    futures.reserve(server_.size() + client_.size());
    futures.clear();

    for (auto& session : server_) { futures.emplace_back(session->Stop()); }

    for (auto& session : client_) { futures.emplace_back(session->Stop()); }

    server_.clear();
    client_.clear();
    auto done{true};

    for (auto& future : futures) {
        using Status = std::future_status;

        if (Status::ready != future.wait_for(1min)) { done = false; }
    }

    if (false == done) {
        LogError()()("shutdown delayed, possibly due to active zmq batches.")
            .Flush();
        LogError()(zmq.Internal().ActiveBatches()).Flush();
    }

    for (auto& future : futures) { future.get(); }
}
}  // namespace opentxs::api

namespace opentxs::api
{
ContextPrivate::ContextPrivate(
    const opentxs::Options& args,
    const opentxs::network::zeromq::Context& zmq,
    const network::Asio& asio,
    const opentxs::internal::ShutdownSender& sender,
    PasswordCaller* password)
    : api::internal::Context()
    , post_(&context_has_terminated)
    , running_(Flag::Factory(true))
    , args_(args)
    , zmq_context_(zmq)
    , asio_(asio)
    , shutdown_sender_(sender)
    , home_(args_.Home().string().c_str())
    , null_callback_(opentxs::Factory::NullCallback())
    , default_external_password_callback_([&] {
        auto out = std::make_unique<PasswordCaller>();

        assert_false(nullptr == out);

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
    , paths_(factory::Paths(home_))
    , config_()
    , crypto_(nullptr)
    , factory_(nullptr)
    , zap_(nullptr)
    , sessions_()
    , rpc_(nullptr)
    , file_lock_()
    , signal_handler_()
    , self_(this)
    , me_()
{
    assert_false(nullptr == null_callback_);
    assert_false(nullptr == default_external_password_callback_);
    assert_true(zmq_context_);
    assert_false(nullptr == external_password_callback_);
    assert_true(external_password_callback_->HaveCallback());
}

auto ContextPrivate::Cancel(TaskID task) const noexcept -> bool
{
    return periodic_->Cancel(task);
}

auto ContextPrivate::client_instance(const int count) -> int
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count);
}

auto ContextPrivate::ClientSession(const int instance) const
    -> const api::session::Client&
{
    const auto& output = sessions_.lock_shared()->client_.at(instance);

    assert_false(nullptr == output);

    return output->asClientPublic();
}

auto ContextPrivate::Config(const std::filesystem::path& path) const noexcept
    -> const api::Settings&
{
    const auto& config = [&]() -> auto& {
        auto handle = config_.lock();
        auto& map = *handle;

        if (auto i = map.find(path); map.end() == i) {
            const auto [out, rc] =
                map.try_emplace(path, factory::Settings(paths_, path));

            assert_true(rc);

            return out->second;
        } else {

            return i->second;
        }
    }();

    return config;
}

auto ContextPrivate::Crypto() const noexcept -> const api::Crypto&
{
    assert_false(nullptr == crypto_);

    return *crypto_;
}

auto ContextPrivate::Factory() const noexcept -> const api::Factory&
{
    assert_false(nullptr == factory_);

    return factory_->Self();
}

auto ContextPrivate::GetPasswordCaller() const noexcept -> PasswordCaller&
{
    assert_false(nullptr == external_password_callback_);

    return *external_password_callback_;
}

auto ContextPrivate::Init(
    std::shared_ptr<const api::internal::Context> me) noexcept -> void
{
    me_ = std::move(me);
    rpc_.reset(opentxs::Factory::RPC(Self()));
    assert_false(nullptr == rpc_);
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

auto ContextPrivate::Init_Crypto() -> void
{
    crypto_ =
        factory::CryptoAPI(Config(paths_.OpentxsConfigFilePath().string()));

    assert_false(nullptr == crypto_);

    paths_.Init(crypto_);
}

auto ContextPrivate::Init_Factory() -> void
{
    factory_ = factory::FactoryAPI(*crypto_);

    assert_false(nullptr == factory_);

    crypto_->Internal().Init(factory_);
}

auto ContextPrivate::Init_Log() -> void
{
    const auto& config = Config(paths_.OpentxsConfigFilePath().string());
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

auto ContextPrivate::Init_Periodic() -> void
{
    periodic_.emplace(asio_);

    assert_true(periodic_.has_value());
}

auto ContextPrivate::init_pid() const -> void
{
    try {
        const auto path = paths_.PIDFilePath();
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

        LogAbort()().Abort();
    }
}

auto ContextPrivate::Init_Profile() -> void
{
    const auto& config = Config(paths_.OpentxsConfigFilePath().string());
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

auto ContextPrivate::Init_Zap() -> void
{
    zap_.reset(opentxs::Factory::ZAP(zmq_context_));

    assert_false(nullptr == zap_);
}

auto ContextPrivate::JobCount() noexcept -> std::atomic<unsigned int>&
{
    static auto count =
        std::atomic<unsigned int>{std::numeric_limits<unsigned int>::max()};

    return count;
}

auto ContextPrivate::NotarySession(const int instance) const
    -> const session::Notary&
{
    const auto& output = sessions_.lock_shared()->server_.at(instance);

    assert_false(nullptr == output);

    return output->asNotaryPublic();
}

auto ContextPrivate::ProfileId() const noexcept -> std::string_view
{
    return profile_id_.get();
}

auto ContextPrivate::RPC(const rpc::request::Base& command) const noexcept
    -> std::unique_ptr<rpc::response::Base>
{
    return rpc_->Process(command);
}

auto ContextPrivate::Reschedule(TaskID task, std::chrono::seconds interval)
    const noexcept -> bool
{
    return periodic_->Reschedule(task, interval);
}

auto ContextPrivate::Schedule(
    std::chrono::seconds interval,
    SimpleCallback task) const noexcept -> TaskID
{
    return periodic_->Schedule(interval, task);
}

auto ContextPrivate::Schedule(
    std::chrono::seconds interval,
    SimpleCallback task,
    std::chrono::seconds last) const noexcept -> TaskID
{
    return periodic_->Schedule(interval, task, last);
}

auto ContextPrivate::server_instance(const int count) -> int
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count) + 1;
}

auto ContextPrivate::Shutdown() noexcept -> void
{
    running_->Off();
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
    factory_.reset();
    config_.lock()->clear();
    me_.reset();
}

auto ContextPrivate::ShuttingDown() const noexcept -> bool
{
    return shutdown_sender_.Activated();
}

auto ContextPrivate::StartClientSession(
    const opentxs::Options& args,
    const int instance) const -> const api::session::Client&
{
    auto handle = sessions_.lock();

    assert_false(handle->shutdown_);
    assert_false(nullptr == me_);

    auto& vector = handle->client_;
    const auto existing = vector.size();
    const auto id = std::max<std::size_t>(0_uz, instance);
    const auto effective = std::min(id, existing);

    if (effective == existing) {
        const auto count = vector.size();

        assert_true(std::numeric_limits<int>::max() > count);

        const auto next = static_cast<int>(count);
        const auto session = client_instance(next);
        auto& client = vector.emplace_back(factory::ClientSession(
            Self(),
            running_.get(),
            args_ + args,
            Config(paths_.ClientConfigFilePath(next).string()),
            *crypto_,
            zmq_context_,
            paths_.ClientDataFolder(next),
            session));

        assert_false(nullptr == client);

        client->asClient().Init();
        client->asClient().Start(client);

        return client->asClientPublic();
    } else {
        const auto& output = vector.at(effective);

        assert_false(nullptr == output);

        return output->asClientPublic();
    }
}

auto ContextPrivate::StartClientSession(const int instance) const
    -> const api::session::Client&
{
    static const auto blank = opentxs::Options{};

    return StartClientSession(blank, instance);
}

auto ContextPrivate::StartClientSession(
    const opentxs::Options& args,
    const int instance,
    std::string_view recoverWords,
    std::string_view recoverPassphrase) const -> const api::session::Client&
{
    assert_true(crypto::HaveHDKeys());
    assert_false(nullptr == me_);

    const auto& client = StartClientSession(args, instance);
    auto reason = client.Factory().PasswordPrompt("Recovering a BIP-39 seed");

    if (0 < recoverWords.size()) {
        auto wordList = me_->Factory().SecretFromText(recoverWords);
        auto phrase = me_->Factory().SecretFromText(recoverPassphrase);
        client.Crypto().Seed().ImportSeed(
            wordList,
            phrase,
            opentxs::crypto::SeedStyle::BIP39,
            opentxs::crypto::Language::en,
            reason);
    }

    return client;
}

auto ContextPrivate::StartNotarySession(
    const opentxs::Options& args,
    const int instance) const -> const session::Notary&
{
    auto handle = sessions_.lock();

    assert_false(handle->shutdown_);
    assert_false(nullptr == me_);

    auto& vector = handle->server_;
    const auto existing = vector.size();
    const auto id = std::max<std::size_t>(0_uz, instance);
    const auto effective = std::min(id, existing);

    if (effective == existing) {
        const auto count = vector.size();

        assert_true(std::numeric_limits<int>::max() > count);

        const auto next = static_cast<int>(count);
        const auto session = server_instance(next);
        auto& server = vector.emplace_back(factory::NotarySession(
            Self(),
            running_,
            args_ + args,
            *crypto_,
            Config(paths_.ServerConfigFilePath(next).string()),
            zmq_context_,
            paths_.ServerDataFolder(next),
            session));

        assert_false(nullptr == server);

        server->asNotary().Start(server);

        return server->asNotaryPublic();
    } else {
        const auto& output = vector.at(effective);

        assert_false(nullptr == output);

        return output->asNotaryPublic();
    }
}

auto ContextPrivate::StartNotarySession(const int instance) const
    -> const session::Notary&
{
    static const auto blank = opentxs::Options{};

    return StartNotarySession(blank, instance);
}

auto ContextPrivate::ZAP() const noexcept -> const api::network::ZAP&
{
    assert_false(nullptr == zap_);

    return *zap_;
}

auto ContextPrivate::ZMQ() const noexcept
    -> const opentxs::network::zeromq::Context&
{
    return zmq_context_;
}

ContextPrivate::~ContextPrivate() = default;
}  // namespace opentxs::api
