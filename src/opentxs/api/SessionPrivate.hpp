// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/Ciphertext.pb.h>
#include <chrono>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>

#include "internal/util/Mutex.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/base/Scheduler.hpp"
#include "opentxs/api/session/base/Storage.hpp"
#include "opentxs/api/session/base/ZMQ.hpp"
#include "opentxs/core/Secret.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Shutdown.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QObject;

namespace opentxs
{
namespace api
{
namespace crypto
{
class Symmetric;
}  // namespace crypto

namespace internal
{
class Network;
class Paths;
}  // namespace internal

namespace session
{
namespace internal
{
class Client;
class Factory;
class Notary;
class Wallet;
}  // namespace internal

class Client;
class Notary;
}  // namespace session

class Crypto;
class Session;
class SessionPrivate;  // IWYU pragma: keep
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

class Flag;
class Options;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::SessionPrivate : virtual public internal::Session,
                                     public session::base::ZMQ,
                                     public session::base::Scheduler,
                                     public session::base::Storage
{
public:
    static auto get_api(const PasswordPrompt& reason) noexcept
        -> const api::Session&;

    auto asClient() const noexcept -> const session::internal::Client& final
    {
        return const_cast<SessionPrivate*>(this)->asClient();
    }
    auto asClientPublic() const noexcept -> const session::Client& final
    {
        return const_cast<SessionPrivate*>(this)->asClientPublic();
    }
    auto asNotary() const noexcept -> const session::internal::Notary& final
    {
        return const_cast<SessionPrivate*>(this)->asNotary();
    }
    auto asNotaryPublic() const noexcept -> const session::Notary& final
    {
        return const_cast<SessionPrivate*>(this)->asNotaryPublic();
    }
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
    auto Instance() const noexcept -> int final { return instance_; }
    auto Paths() const noexcept -> const api::internal::Paths& final;
    auto Lock() const -> std::mutex& final { return master_key_lock_; }
    auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::symmetric::Key& final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void override {}
    auto Network() const noexcept -> const api::Network& final
    {
        return network_;
    }
    auto QtRootObject() const noexcept -> QObject* final
    {
        return parent_.QtRootObject();
    }
    auto SetMasterKeyTimeout(const std::chrono::seconds& timeout) const noexcept
        -> void final;
    auto ShuttingDown() const noexcept -> bool final;
    auto Stop() noexcept -> std::future<void> final;
    auto Storage() const noexcept -> const api::session::Storage& final;
    auto Wallet() const noexcept -> const session::Wallet& final;

    auto asClient() noexcept -> session::internal::Client& override;
    auto asClientPublic() noexcept -> session::Client& override;
    auto asNotary() noexcept -> session::internal::Notary& override;
    auto asNotaryPublic() noexcept -> session::Notary& override;
    auto MasterKey(const opentxs::Lock& lock)
        -> opentxs::crypto::symmetric::Key&;

    SessionPrivate() = delete;
    SessionPrivate(const SessionPrivate&) = delete;
    SessionPrivate(SessionPrivate&&) = delete;
    auto operator=(const SessionPrivate&) -> SessionPrivate& = delete;
    auto operator=(SessionPrivate&&) -> SessionPrivate& = delete;

    ~SessionPrivate() override;

protected:
    api::Network network_;
    opentxs::internal::ShutdownSender shutdown_sender_;
    std::unique_ptr<api::session::internal::Wallet> wallet_;

private:
    protobuf::Ciphertext encrypted_secret_;

protected:
    using NetworkMaker = std::function<api::internal::Network*(
        const opentxs::network::zeromq::Context& zmq,
        const api::session::Endpoints& endpoints,
        api::session::base::Scheduler& config)>;

    static auto make_master_key(
        const api::Context& parent,
        const api::session::Factory& factory,
        protobuf::Ciphertext& encrypted_secret_,
        std::optional<Secret>& master_secret_,
        const api::crypto::Symmetric& symmetric,
        const api::session::Storage& storage)
        -> opentxs::crypto::symmetric::Key;

    auto wait_for_init() const noexcept -> void { return init_.get(); }

    auto cleanup() noexcept -> void final;
    // NOTE call from final destructor bodies
    auto shutdown_complete() noexcept -> void;
    auto start(std::shared_ptr<const internal::Session> api) noexcept -> void;

    SessionPrivate(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& zmq,
        const std::filesystem::path& dataFolder,
        const int instance,
        NetworkMaker network,
        std::unique_ptr<api::session::internal::Factory> factory);

private:
    mutable std::mutex master_key_lock_;
    mutable std::optional<Secret> master_secret_;
    mutable std::optional<opentxs::crypto::symmetric::Key> master_key_;
    mutable std::chrono::seconds password_duration_;
    mutable Time last_activity_;
    std::promise<void> init_promise_;
    const std::shared_future<void> init_;
    std::promise<void> shutdown_promise_;

    void bump_password_timer(const opentxs::Lock& lock) const;
    // TODO void password_timeout() const;
};
