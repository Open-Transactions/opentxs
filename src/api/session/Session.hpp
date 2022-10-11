// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Ciphertext.pb.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#include "api/session/base/Scheduler.hpp"
#include "api/session/base/Storage.hpp"
#include "api/session/base/ZMQ.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/storage/Config.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QObject;

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto

namespace session
{
class Crypto;
class Factory;
}  // namespace session

class Context;
class Crypto;
class Legacy;
class Session;
class Settings;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Ciphertext;
}  // namespace proto

class Flag;
class Options;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class Session : virtual public internal::Session,
                public ZMQ,
                public Scheduler,
                public base::Storage
{
public:
    static auto get_api(const PasswordPrompt& reason) noexcept
        -> const api::Session&;

    auto Config() const noexcept -> const api::Settings& final
    {
        return config_;
    }
    auto Crypto() const noexcept -> const session::Crypto& final
    {
        return crypto_;
    }
    auto DataFolder() const noexcept -> const std::filesystem::path& final
    {
        return data_folder_;
    }
    auto Endpoints() const noexcept -> const session::Endpoints& final
    {
        return endpoints_;
    }
    auto Factory() const noexcept -> const session::Factory& final
    {
        return factory_;
    }
    auto GetInternalPasswordCallback() const
        -> INTERNAL_PASSWORD_CALLBACK* final;
    auto GetOptions() const noexcept -> const Options& final { return args_; }
    auto GetSecret(
        const opentxs::Lock& lock,
        Secret& secret,
        const PasswordPrompt& reason,
        const bool twice,
        const UnallocatedCString& key) const -> bool final;
    auto GetShared() const noexcept
        -> std::shared_ptr<const api::Session> final;
    auto Instance() const noexcept -> int final { return instance_; }
    auto Legacy() const noexcept -> const api::Legacy& final;
    auto Lock() const -> std::mutex& final { return master_key_lock_; }
    auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::key::Symmetric& final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void override {}
    auto Network() const noexcept -> const network::Network& final
    {
        return *network_;
    }
    auto QtRootObject() const noexcept -> QObject* final
    {
        return parent_.QtRootObject();
    }
    auto SetMasterKeyTimeout(const std::chrono::seconds& timeout) const noexcept
        -> void final;
    auto ShuttingDown() const noexcept -> bool final;
    auto Start(std::shared_ptr<const api::Session> api) noexcept
        -> void override;
    auto Stop() noexcept -> std::future<void> final;
    auto Storage() const noexcept -> const api::session::Storage& final;
    auto Wallet() const noexcept -> const session::Wallet& final;

    Session() = delete;
    Session(const Session&) = delete;
    Session(Session&&) = delete;
    auto operator=(const Session&) -> Session& = delete;
    auto operator=(Session&&) -> Session& = delete;

    ~Session() override;

protected:
    std::unique_ptr<network::Network> network_;
    opentxs::internal::ShutdownSender shutdown_sender_;
    std::unique_ptr<api::session::Wallet> wallet_;

private:
    proto::Ciphertext encrypted_secret_;

protected:
    using NetworkMaker = std::function<std::unique_ptr<api::network::Network>(
        const opentxs::network::zeromq::Context& zmq,
        const api::session::Endpoints& endpoints,
        api::session::Scheduler& config)>;

    static auto make_master_key(
        const api::Context& parent,
        const api::session::Factory& factory,
        proto::Ciphertext& encrypted_secret_,
        std::optional<Secret>& master_secret_,
        const api::crypto::Symmetric& symmetric,
        const api::session::Storage& storage) -> OTSymmetricKey;

    auto cleanup() noexcept -> void final;
    // NOTE call from final destructor bodies
    auto shutdown_complete() noexcept -> void;

    Session(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& zmq,
        const std::filesystem::path& dataFolder,
        const int instance,
        NetworkMaker network,
        std::unique_ptr<api::session::Factory> factory);

private:
    mutable std::mutex master_key_lock_;
    mutable std::optional<Secret> master_secret_;
    mutable OTSymmetricKey master_key_;
    mutable std::chrono::seconds password_duration_;
    mutable Time last_activity_;
    std::promise<void> init_promise_;
    const std::shared_future<void> init_;
    std::promise<void> shutdown_promise_;
    std::weak_ptr<const api::Session> self_;

    void bump_password_timer(const opentxs::Lock& lock) const;
    // TODO void password_timeout() const;

    void storage_gc_hook() final;
};
}  // namespace opentxs::api::session::imp
