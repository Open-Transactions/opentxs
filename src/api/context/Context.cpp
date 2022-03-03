// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "api/context/Context.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>  // IWYU pragma: keep
#include <functional>
#include <iosfwd>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/Crypto.hpp"
#include "internal/api/Factory.hpp"
#include "internal/api/Log.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/interface/rpc/RPC.hpp"
#include "internal/network/zeromq/Factory.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Log.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Signals.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/PasswordCallback.hpp"
#include "opentxs/util/PasswordCaller.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
auto Context(
    Flag& running,
    const Options& args,
    PasswordCaller* externalPasswordCallback) noexcept
    -> std::unique_ptr<api::internal::Context>
{
    using ReturnType = api::imp::Context;

    return std::make_unique<ReturnType>(
        running, args, externalPasswordCallback);
}
}  // namespace opentxs::factory

namespace opentxs::api
{
auto Context::PrepareSignalHandling() noexcept -> void { Signals::Block(); }

auto Context::SuggestFolder(const UnallocatedCString& app) noexcept
    -> UnallocatedCString
{
    return Legacy::SuggestFolder(app);
}
}  // namespace opentxs::api

namespace opentxs::api::imp
{
Context::Context(
    Flag& running,
    const Options& args,
    PasswordCaller* externalPasswordCallback)
    : api::internal::Context()
    , Lockable()
    , Periodic(running)
    , args_(args)
    , home_(args_.Home())
    , config_lock_()
    , task_list_lock_()
    , signal_handler_lock_()
    , config_()
    , zmq_context_(opentxs::factory::ZMQContext())
    , signal_handler_(nullptr)
    , log_(factory::Log(*zmq_context_, args_.RemoteLogEndpoint()))
    , asio_()
    , crypto_(nullptr)
    , factory_(nullptr)
    , legacy_(factory::Legacy(home_))
    , zap_(nullptr)
    , profile_id_()
    , shutdown_callback_(nullptr)
    , null_callback_(nullptr)
    , default_external_password_callback_(nullptr)
    , external_password_callback_{externalPasswordCallback}
    , file_lock_()
    , server_()
    , client_()
    , rpc_(opentxs::Factory::RPC(*this))
{
    // NOTE: OT_ASSERT is not available until Init() has been called
    assert(zmq_context_);
    assert(legacy_);
    assert(log_);

    if (nullptr == external_password_callback_) {
        setup_default_external_password_callback();
    }

    assert(nullptr != external_password_callback_);
    assert(rpc_);
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
    auto& output = client_.at(instance);

    OT_ASSERT(output);

    return *output;
}

auto Context::Config(const UnallocatedCString& path) const noexcept
    -> const api::Settings&
{
    std::unique_lock<std::mutex> lock(config_lock_);
    auto& config = config_[path];

    if (!config) {
        config = factory::Settings(*legacy_, String::Factory(path));
    }

    OT_ASSERT(config);

    lock.unlock();

    return *config;
}

auto Context::Crypto() const noexcept -> const api::Crypto&
{
    OT_ASSERT(crypto_)

    return *crypto_;
}

auto Context::Factory() const noexcept -> const api::Factory&
{
    OT_ASSERT(factory_)

    return *factory_;
}

auto Context::GetPasswordCaller() const noexcept -> PasswordCaller&
{
    OT_ASSERT(nullptr != external_password_callback_)

    return *external_password_callback_;
}

auto Context::ProfileId() const noexcept -> UnallocatedCString
{
    return profile_id_;
}

auto Context::Init() noexcept -> void
{
    Init_Log();
    Init_Asio();
    init_pid();
    Init_Crypto();
    Init_Factory();
    Init_Profile();
    Init_Rlimit();
    Init_Zap();
}

auto Context::Init_Asio() -> void
{
    asio_ = std::make_unique<network::Asio>(*zmq_context_);

    OT_ASSERT(asio_);

    asio_->Init();
}

auto Context::Init_Crypto() -> void
{
    crypto_ = factory::CryptoAPI(Config(legacy_->OpentxsConfigFilePath()));

    OT_ASSERT(crypto_);
}

auto Context::Init_Profile() -> void
{
    const auto& config = Config(legacy_->OpentxsConfigFilePath());
    bool profile_id_exists{false};
    auto existing_profile_id{String::Factory()};
    config.Check_str(
        String::Factory("profile"),
        String::Factory("profile_id"),
        existing_profile_id,
        profile_id_exists);
    if (profile_id_exists) {
        profile_id_ = UnallocatedCString(existing_profile_id->Get());
    } else {
        const auto new_profile_id(crypto_->Encode().Nonce(20));
        bool new_or_update{true};
        config.Set_str(
            String::Factory("profile"),
            String::Factory("profile_id"),
            new_profile_id,
            new_or_update);
        profile_id_ = UnallocatedCString(new_profile_id->Get());
    }
}

auto Context::Init_Factory() -> void
{
    factory_ = factory::FactoryAPI(*crypto_);

    OT_ASSERT(factory_);

    crypto_->Internal().Init(*factory_);
}

auto Context::Init_Log() -> void
{
    OT_ASSERT(legacy_)

    const auto& config = Config(legacy_->OpentxsConfigFilePath());
    auto notUsed{false};
    auto level = std::int64_t{0};
    const auto value = args_.LogLevel();

    if (-1 > value) {
        config.CheckSet_long(
            String::Factory("logging"),
            String::Factory("log_level"),
            0,
            level,
            notUsed);
    } else {
        config.Set_long(
            String::Factory("logging"),
            String::Factory("log_level"),
            value,
            notUsed);
        level = value;
    }

    opentxs::internal::Log::SetVerbosity(static_cast<int>(level));
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

auto Context::Init_Zap() -> void
{
    zap_.reset(opentxs::Factory::ZAP(*zmq_context_));

    OT_ASSERT(zap_);
}

auto Context::NotarySession(const int instance) const -> const session::Notary&
{
    auto& output = server_.at(instance);

    OT_ASSERT(output);

    return *output;
}

auto Context::RPC(const rpc::request::Base& command) const noexcept
    -> std::unique_ptr<rpc::response::Base>
{
    return rpc_->Process(command);
}

auto Context::server_instance(const int count) -> int
{
    // NOTE: Instance numbers must not collide between clients and servers.
    // Clients use even numbers and servers use odd numbers.
    return (2 * count) + 1;
}

auto Context::setup_default_external_password_callback() -> void
{
    // NOTE: OT_ASSERT is not available yet because we're too early
    // in the startup process

    null_callback_.reset(opentxs::Factory::NullCallback());

    assert(null_callback_);

    default_external_password_callback_ = std::make_unique<PasswordCaller>();

    assert(default_external_password_callback_);

    default_external_password_callback_->SetCallback(null_callback_.get());

    assert(default_external_password_callback_->HaveCallback());

    external_password_callback_ = default_external_password_callback_.get();
}

auto Context::shutdown() noexcept -> void
{
    running_.Off();
    Periodic::Shutdown();

    if (nullptr != shutdown_callback_) {
        ShutdownCallback& callback = *shutdown_callback_;
        callback();
        shutdown_callback_ = nullptr;
    }

    rpc_.reset();
    server_.clear();
    zap_.reset();
    client_.clear();
    shutdown_qt();
    crypto_.reset();
    legacy_.reset();
    factory_.reset();

    for (auto& config : config_) { config.second.reset(); }

    config_.clear();

    if (asio_) {
        asio_->Shutdown();
        asio_.reset();
    }

    opentxs::internal::Log::Shutdown();
    log_.reset();
}

auto Context::start_client(const Lock& lock, const Options& args) const -> void
{
    OT_ASSERT(verify_lock(lock))
    OT_ASSERT(crypto_);
    OT_ASSERT(legacy_);
    OT_ASSERT(std::numeric_limits<int>::max() > client_.size());

    const auto next = static_cast<int>(client_.size());
    const auto instance = client_instance(next);
    auto& client = client_.emplace_back(factory::ClientSession(
        *this,
        running_,
        args_ + args,
        Config(legacy_->ClientConfigFilePath(next)),
        *crypto_,
        *zmq_context_,
        legacy_->ClientDataFolder(next),
        instance));

    OT_ASSERT(client);

    client->InternalClient().Init();
}

auto Context::StartClientSession(const Options& args, const int instance) const
    -> const api::session::Client&
{
    auto lock = Lock{lock_};

    const auto count = std::max<std::size_t>(0u, instance);
    const auto effective = std::min<std::size_t>(count, client_.size());

    if (effective == client_.size()) { start_client(lock, args); }

    const auto& output = client_.at(effective);

    OT_ASSERT(output)

    return *output;
}

auto Context::StartClientSession(const int instance) const
    -> const api::session::Client&
{
    static const auto blank = Options{};

    return StartClientSession(blank, instance);
}

auto Context::StartClientSession(
    const Options& args,
    const int instance,
    const UnallocatedCString& recoverWords,
    const UnallocatedCString& recoverPassphrase) const
    -> const api::session::Client&
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

auto Context::start_server(const Lock& lock, const Options& args) const -> void
{
    OT_ASSERT(verify_lock(lock));
    OT_ASSERT(crypto_);
    OT_ASSERT(std::numeric_limits<int>::max() > server_.size());

    const auto next = static_cast<int>(server_.size());
    const auto instance{server_instance(next)};
    server_.emplace_back(factory::NotarySession(
        *this,
        running_,
        args_ + args,
        *crypto_,
        Config(legacy_->ServerConfigFilePath(next)),
        *zmq_context_,
        legacy_->ServerDataFolder(next),
        instance));
}

auto Context::StartNotarySession(const Options& args, const int instance) const
    -> const session::Notary&
{
    auto lock = Lock{lock_};

    OT_ASSERT(std::numeric_limits<int>::max() > server_.size());

    const auto size = static_cast<int>(server_.size());
    const auto count = std::max<int>(0, instance);
    const auto effective = std::min(count, size);

    if (effective == size) { start_server(lock, args); }

    const auto& output = server_.at(effective);

    OT_ASSERT(output)

    return *output;
}

auto Context::StartNotarySession(const int instance) const
    -> const session::Notary&
{
    static const auto blank = Options{};

    return StartNotarySession(blank, instance);
}

auto Context::ZAP() const noexcept -> const api::network::ZAP&
{
    OT_ASSERT(zap_);

    return *zap_;
}

Context::~Context() { shutdown(); }
}  // namespace opentxs::api::imp
