// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/Context.hpp"

#pragma once

#include <boost/interprocess/sync/file_lock.hpp>
#include <cs_ordered_guarded.h>
#include <cs_plain_guarded.h>
#include <cs_shared_guarded.h>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string_view>

#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Signals.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Context.internal.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Paths.internal.hpp"
#include "opentxs/api/PeriodicPrivate.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Types.hpp"
#include "util/ScopeGuard.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QObject;

namespace opentxs
{
namespace api
{
namespace internal
{
class Factory;
}  // namespace internal

namespace network
{
class Asio;
}  // namespace network

namespace session
{
namespace internal
{
class Client;
class Notary;
}  // namespace internal
}  // namespace session

class ContextPrivate;  // IWYU pragma: keep
}  // namespace api

namespace internal
{
class ShutdownSender;
}  // namespace internal

namespace network
{
namespace zeromq
{
class Context;  // IWYU pragma: keep
}  // namespace zeromq
}  // namespace network

namespace rpc
{
namespace internal
{
struct RPC;
}  // namespace internal

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc

class PasswordCallback;
class PasswordCaller;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

extern "C" {
struct rlimit;
}

class opentxs::api::ContextPrivate final : public internal::Context
{
public:
    static auto JobCount() noexcept -> std::atomic<unsigned int>&;

    auto Asio() const noexcept -> const network::Asio& final { return asio_; }
    auto Cancel(TaskID) const noexcept -> bool final;
    auto ClientSession(const int instance) const noexcept(false)
        -> const api::session::Client& final;
    auto ClientSessionCount() const noexcept -> std::size_t final
    {
        return sessions_.lock_shared()->client_.size();
    }
    auto Config(const std::filesystem::path& path) const noexcept
        -> const api::Settings& final;
    auto Crypto() const noexcept -> const api::Crypto& final;
    auto Factory() const noexcept -> const api::Factory& final;
    auto GetPasswordCaller() const noexcept -> PasswordCaller& final;
    auto HandleSignals(SimpleCallback* shutdown) const noexcept -> void final;
    auto Paths() const noexcept -> const api::internal::Paths& final
    {
        return paths_;
    }
    auto NotarySession(const int instance) const noexcept(false)
        -> const api::session::Notary& final;
    auto NotarySessionCount() const noexcept -> std::size_t final
    {
        return sessions_.lock_shared()->server_.size();
    }
    auto Options() const noexcept -> const opentxs::Options& final
    {
        return args_;
    }
    auto ProfileId() const noexcept -> std::string_view final;
    auto QtRootObject(QObject* parent) const noexcept -> QObject* final;
    auto Reschedule(TaskID, std::chrono::seconds interval) const noexcept
        -> bool final;
    auto RPC(const rpc::request::Base& command) const noexcept
        -> std::unique_ptr<rpc::response::Base> final;
    auto RPC(const ReadView command, Writer&& response) const noexcept
        -> bool final;
    auto Schedule(std::chrono::seconds interval, SimpleCallback task)
        const noexcept -> TaskID final;
    auto Schedule(
        std::chrono::seconds interval,
        SimpleCallback task,
        std::chrono::seconds last) const noexcept -> TaskID final;
    auto Self() const noexcept -> const api::Context& final { return self_; }
    auto ShuttingDown() const noexcept -> bool final;
    auto StartClientSession(const opentxs::Options& args, const int instance)
        const -> const api::session::Client& final;
    auto StartClientSession(const int instance) const
        -> const api::session::Client& final;
    auto StartClientSession(
        const opentxs::Options& args,
        const int instance,
        std::string_view recoverWords,
        std::string_view recoverPassphrase) const
        -> const api::session::Client& final;
    auto StartNotarySession(const opentxs::Options& args, const int instance)
        const -> const session::Notary& final;
    auto StartNotarySession(const int instance) const
        -> const session::Notary& final;
    auto ZAP() const noexcept -> const api::network::ZAP& final;
    auto ZMQ() const noexcept -> const opentxs::network::zeromq::Context& final;

    auto Init(std::shared_ptr<const api::internal::Context> me) noexcept
        -> void final;
    auto Self() noexcept -> api::Context& final { return self_; }
    auto Shutdown() noexcept -> void final;

    ContextPrivate(
        const opentxs::Options& args,
        const opentxs::network::zeromq::Context& zmq,
        const network::Asio& asio,
        const opentxs::internal::ShutdownSender& sender,
        PasswordCaller* externalPasswordCallback = nullptr);
    ContextPrivate() = delete;
    ContextPrivate(const ContextPrivate&) = delete;
    ContextPrivate(ContextPrivate&&) = delete;
    auto operator=(const ContextPrivate&) -> ContextPrivate& = delete;
    auto operator=(ContextPrivate&&) -> ContextPrivate& = delete;

    ~ContextPrivate() final;

private:
    struct SignalHandler {
        SimpleCallback* callback_{nullptr};
        std::unique_ptr<Signals> handler_{};
    };
    struct Sessions {
        bool shutdown_{false};
        Vector<std::shared_ptr<api::session::internal::Notary>> server_{};
        Vector<std::shared_ptr<api::session::internal::Client>> client_{};

        auto clear(const opentxs::network::zeromq::Context& zmq) noexcept
            -> void;
    };

    using ConfigMap = Map<std::filesystem::path, api::Settings>;
    using GuardedConfig = libguarded::plain_guarded<ConfigMap>;
    using GuardedSessions =
        libguarded::shared_guarded<Sessions, std::shared_mutex>;
    using GuardedSignals =
        libguarded::ordered_guarded<SignalHandler, std::shared_mutex>;

    ScopeGuard post_;
    mutable OTFlag running_;
    const opentxs::Options args_;
    const opentxs::network::zeromq::Context& zmq_context_;
    const network::Asio& asio_;
    const opentxs::internal::ShutdownSender& shutdown_sender_;
    const std::filesystem::path home_;
    const std::unique_ptr<PasswordCallback> null_callback_;
    const std::unique_ptr<PasswordCaller> default_external_password_callback_;
    PasswordCaller* const external_password_callback_;
    DeferredConstruction<CString> profile_id_;
    std::optional<api::PeriodicPrivate> periodic_;
    api::internal::Paths paths_;
    mutable GuardedConfig config_;
    std::shared_ptr<api::Crypto> crypto_;
    std::shared_ptr<api::internal::Factory> factory_;
    std::unique_ptr<api::network::ZAP> zap_;
    mutable GuardedSessions sessions_;
    std::unique_ptr<rpc::internal::RPC> rpc_;
    mutable boost::interprocess::file_lock file_lock_;
    mutable GuardedSignals signal_handler_;
    api::Context self_;
    std::shared_ptr<const api::internal::Context> me_;

    static auto client_instance(const int count) -> int;
    static auto server_instance(const int count) -> int;
    static auto set_desired_files(::rlimit& out) noexcept -> void;

    auto init_pid() const -> void;

    auto get_qt() const noexcept -> std::unique_ptr<QObject>&;
    auto Init_CoreDump() noexcept -> void;
    auto Init_Crypto() -> void;
    auto Init_Factory() -> void;
    auto Init_Log() -> void;
    auto Init_Periodic() -> void;
    auto Init_Profile() -> void;
    auto Init_Rlimit() noexcept -> void;
    auto Init_Zap() -> void;
    auto shutdown_qt() noexcept -> void;
};
