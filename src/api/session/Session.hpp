// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#include "api/session/base/Scheduler.hpp"
#include "api/session/base/Storage.hpp"
#include "api/session/base/ZMQ.hpp"
#include "internal/api/session/Session.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Dht.hpp"
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
#include "serialization/protobuf/Ciphertext.pb.h"
#include "util/storage/Config.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QObject;

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class OTPassword;
class Options;
class PasswordPrompt;
// }  // namespace v1
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
    auto DataFolder() const noexcept -> const UnallocatedCString& final
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
    auto Legacy() const noexcept -> const api::Legacy& final;
    auto Lock() const -> std::mutex& final { return master_key_lock_; }
    auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::key::Symmetric& final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void override {}
    auto Network() const noexcept -> const network::Network& final
    {
        return network_;
    }
    auto QtRootObject() const noexcept -> QObject* final
    {
        return parent_.QtRootObject();
    }
    auto SetMasterKeyTimeout(const std::chrono::seconds& timeout) const noexcept
        -> void final;
    auto Storage() const noexcept -> const api::session::Storage& final;
    auto Wallet() const noexcept -> const session::Wallet& final;

    ~Session() override;

protected:
    network::Network network_;
    std::unique_ptr<api::session::Wallet> wallet_;

private:
    proto::Ciphertext encrypted_secret_;

protected:
    using NetworkMaker = std::function<api::network::Network::Imp*(
        const opentxs::network::zeromq::Context& zmq,
        const api::session::Endpoints& endpoints,
        api::session::Scheduler& config)>;

    static auto make_master_key(
        const api::Context& parent,
        const api::session::Factory& factory,
        proto::Ciphertext& encrypted_secret_,
        std::optional<OTSecret>& master_secret_,
        const api::crypto::Symmetric& symmetric,
        const api::session::Storage& storage) -> OTSymmetricKey;

    auto cleanup() noexcept -> void final;

    Session(
        const api::Context& parent,
        Flag& running,
        Options&& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& zmq,
        const UnallocatedCString& dataFolder,
        const int instance,
        NetworkMaker network,
        std::unique_ptr<api::session::Factory> factory);

private:
    mutable std::mutex master_key_lock_;
    mutable std::optional<OTSecret> master_secret_;
    mutable OTSymmetricKey master_key_;
    mutable std::chrono::seconds password_duration_;
    mutable Time last_activity_;

    void bump_password_timer(const opentxs::Lock& lock) const;
    // TODO void password_timeout() const;

    void storage_gc_hook() final;

    Session() = delete;
    Session(const Session&) = delete;
    Session(Session&&) = delete;
    auto operator=(const Session&) -> Session& = delete;
    auto operator=(Session&&) -> Session& = delete;
};
}  // namespace opentxs::api::session::imp
