// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>

#include "opentxs/Types.hpp"
#include "opentxs/api/Periodic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QObject;

namespace opentxs
{
namespace api
{
namespace internal
{
class Context;  // IWYU pragma: keep
class Paths;
}  // namespace internal

namespace network
{
class Asio;
class ZAP;
}  // namespace network

namespace session
{
class Client;
class Notary;
}  // namespace session

class Context;
class Crypto;
class Factory;
class Settings;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace rpc
{
namespace request
{
class Message;
}  // namespace request

namespace response
{
class Message;
}  // namespace response
}  // namespace rpc

class Options;
class PasswordCaller;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::internal::Context : virtual public Periodic
{
public:
    static auto MaxJobs() noexcept -> unsigned int;
    static auto SetMaxJobs(const opentxs::Options& args) noexcept -> void;

    virtual auto Asio() const noexcept -> const network::Asio& = 0;
    virtual auto ClientSession(const int instance) const noexcept(false)
        -> const api::session::Client& = 0;
    virtual auto ClientSessionCount() const noexcept -> std::size_t = 0;
    virtual auto Config(const std::filesystem::path& path) const noexcept
        -> const api::Settings& = 0;
    virtual auto Crypto() const noexcept -> const api::Crypto& = 0;
    virtual auto Factory() const noexcept -> const api::Factory& = 0;
    virtual auto GetPasswordCaller() const noexcept -> PasswordCaller& = 0;
    virtual auto HandleSignals(SimpleCallback* callback) const noexcept
        -> void = 0;
    virtual auto Paths() const noexcept -> const api::internal::Paths& = 0;
    virtual auto NotarySession(const int instance) const noexcept(false)
        -> const session::Notary& = 0;
    virtual auto NotarySessionCount() const noexcept -> std::size_t = 0;
    virtual auto Options() const noexcept -> const opentxs::Options& = 0;
    virtual auto ProfileId() const noexcept -> std::string_view = 0;
    virtual auto QtRootObject(QObject* parent = nullptr) const noexcept
        -> QObject* = 0;
    virtual auto RPC(const ReadView command, Writer&& response) const noexcept
        -> bool = 0;
    virtual auto RPC(const rpc::request::Message& command) const noexcept
        -> std::unique_ptr<rpc::response::Message> = 0;
    virtual auto Self() const noexcept -> const api::Context& = 0;
    virtual auto ShuttingDown() const noexcept -> bool = 0;
    virtual auto StartClientSession(const int instance) const
        -> const api::session::Client& = 0;
    virtual auto StartClientSession(
        const opentxs::Options& args,
        const int instance) const -> const api::session::Client& = 0;
    virtual auto StartClientSession(
        const opentxs::Options& args,
        const int instance,
        std::string_view recoverWords,
        std::string_view recoverPassphrase) const
        -> const api::session::Client& = 0;
    virtual auto StartNotarySession(const int instance) const
        -> const session::Notary& = 0;
    virtual auto StartNotarySession(
        const opentxs::Options& args,
        const int instance) const -> const session::Notary& = 0;
    virtual auto ZAP() const noexcept -> const api::network::ZAP& = 0;
    virtual auto ZMQ() const noexcept
        -> const opentxs::network::zeromq::Context& = 0;

    virtual auto Init(std::shared_ptr<const api::internal::Context> me) noexcept
        -> void = 0;
    virtual auto Self() noexcept -> api::Context& = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    ~Context() override = default;
};

namespace opentxs
{
auto context_has_terminated() noexcept -> void;
auto get_context_for_unit_tests() noexcept
    -> std::shared_ptr<const api::internal::Context>;
auto get_zeromq() noexcept -> std::weak_ptr<const network::zeromq::Context>;
auto zmq_has_terminated() noexcept -> void;
}  // namespace opentxs
