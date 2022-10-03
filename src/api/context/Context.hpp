// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/Context.hpp"

#pragma once

#include <RPCResponse.pb.h>
#include <boost/interprocess/sync/file_lock.hpp>
#include <cs_ordered_guarded.h>
#include <cs_plain_guarded.h>
#include <cs_shared_guarded.h>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string_view>

#include "Proto.hpp"
#include "api/Periodic.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/Legacy.hpp"
#include "internal/util/AsyncConst.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Signals.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/interface/rpc/request/Base.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace internal
{
class ShutdownSender;
}  // namespace internal

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class RPCCommand;
}  // namespace proto

namespace rpc
{
namespace internal
{
struct RPC;
}  // namespace internal

namespace request
{
class Base;
}  // namespace request

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc

class Flag;
class PasswordCallback;
class PasswordCaller;
class Signals;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

extern "C" {
struct rlimit;
}

namespace opentxs::api::imp
{
class Context final : public internal::Context
{
public:
    static auto JobCount() noexcept -> std::atomic<unsigned int>&;

    auto Asio() const noexcept -> const network::Asio& final { return asio_; }
    auto Cancel(const TaskID task) const -> bool final;
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
    auto HandleSignals(ShutdownCallback* shutdown) const noexcept -> void final;
    auto Legacy() const noexcept -> const api::Legacy& final
    {
        return *legacy_;
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
    auto QtRootObject() const noexcept -> QObject* final;
    auto Reschedule(const TaskID task, const std::chrono::seconds& interval)
        const -> bool final;
    auto RPC(const rpc::request::Base& command) const noexcept
        -> std::unique_ptr<rpc::response::Base> final;
    auto RPC(const ReadView command, const AllocateOutput response)
        const noexcept -> bool final;
    auto Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last) const -> TaskID final;
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

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> void final;

    Context(
        const opentxs::Options& args,
        const opentxs::network::zeromq::Context& zmq,
        const network::Asio& asio,
        const opentxs::internal::ShutdownSender& sender,
        Flag& running,
        std::promise<void>& shutdown,
        PasswordCaller* externalPasswordCallback = nullptr);
    Context() = delete;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;

    ~Context() final;

private:
    struct SignalHandler {
        ShutdownCallback* callback_{nullptr};
        std::unique_ptr<Signals> handler_{};
    };
    struct Sessions {
        bool shutdown_{false};
        Vector<std::shared_ptr<api::session::Notary>> server_{};
        Vector<std::shared_ptr<api::session::Client>> client_{};

        auto clear(const opentxs::network::zeromq::Context& zmq) noexcept
            -> void;
    };

    using ConfigMap =
        Map<std::filesystem::path, std::unique_ptr<api::Settings>>;
    using GuardedConfig = libguarded::plain_guarded<ConfigMap>;
    using GuardedSessions =
        libguarded::shared_guarded<Sessions, std::shared_mutex>;
    using GuardedSignals =
        libguarded::ordered_guarded<SignalHandler, std::shared_mutex>;

    Flag& running_;
    std::promise<void>& shutdown_;
    const opentxs::Options args_;
    const opentxs::network::zeromq::Context& zmq_context_;
    const network::Asio& asio_;
    const opentxs::internal::ShutdownSender& shutdown_sender_;
    const std::filesystem::path home_;
    const std::unique_ptr<PasswordCallback> null_callback_;
    const std::unique_ptr<PasswordCaller> default_external_password_callback_;
    PasswordCaller* const external_password_callback_;
    AsyncConst<CString> profile_id_;
    std::optional<api::imp::Periodic> periodic_;
    std::unique_ptr<api::Legacy> legacy_;
    mutable GuardedConfig config_;
    std::unique_ptr<api::Crypto> crypto_;
    std::shared_ptr<api::Factory> factory_;
    std::unique_ptr<api::network::ZAP> zap_;
    mutable GuardedSessions sessions_;
    std::unique_ptr<rpc::internal::RPC> rpc_;
    mutable boost::interprocess::file_lock file_lock_;
    mutable GuardedSignals signal_handler_;

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
}  // namespace opentxs::api::imp
