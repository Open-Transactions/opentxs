// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string_view>

#include "opentxs/Export.hpp"
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
class Context;
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

class Context;  // IWYU pragma: keep
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
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 The top-level Context for the OT API. Child class of Periodic.
 Both Client and Server contexts are derived from this class.
 */
class OPENTXS_EXPORT opentxs::api::Context final : public Periodic
{
public:
    /** NOTE You must call PrepareSignalHandling() prior to initializing the
     * context if you intend to use signal handling */
    static auto PrepareSignalHandling() noexcept -> void;
    static auto SuggestFolder(std::string_view appName) noexcept
        -> std::filesystem::path;

    /// Returns a handle to the ASIO API.
    auto Asio() const noexcept -> const network::Asio&;
    auto Cancel(TaskID task) const noexcept -> bool final;
    /** Throws std::out_of_range if the specified session does not exist. */
    auto ClientSession(const int instance) const noexcept(false)
        -> const api::session::Client&;
    /// Returns the number of client sessions.
    auto ClientSessionCount() const noexcept -> std::size_t;
    /// Returns the settings for a given config file.
    auto Config(const std::filesystem::path& path) const noexcept
        -> const api::Settings&;
    /// Returns a handle to the top-level crypto API.
    auto Crypto() const noexcept -> const api::Crypto&;
    /// Returns a handle to the top-level Factory API.
    auto Factory() const noexcept -> const api::Factory&;
    /** WARNING You must call PrepareSignalHandling() prior to initializing
     * the context if you intend to use this function */
    auto HandleSignals(SimpleCallback* callback = nullptr) const noexcept
        -> void;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Context&;
    /** Throws std::out_of_range if the specified session does not exist. */
    auto NotarySession(const int instance) const noexcept(false)
        -> const session::Notary&;
    /// Returns a count of the notary sessions.
    auto NotarySessionCount() const noexcept -> std::size_t;
    auto Options() const noexcept -> const opentxs::Options&;
    auto ProfileId() const noexcept -> std::string_view;
    OPENTXS_NO_EXPORT auto QtRootObject(
        QObject* parent = nullptr) const noexcept -> QObject*;
    auto Reschedule(TaskID task, std::chrono::seconds interval) const noexcept
        -> bool final;
    /// Used for sending RPC requests. Returns RPC response.
    auto RPC(const rpc::request::Message& command) const noexcept
        -> std::unique_ptr<rpc::response::Message>;
    auto RPC(const ReadView command, Writer&& response) const noexcept -> bool;
    auto Schedule(std::chrono::seconds interval, opentxs::SimpleCallback task)
        const noexcept -> TaskID final;
    auto Schedule(
        std::chrono::seconds interval,
        opentxs::SimpleCallback task,
        std::chrono::seconds last) const noexcept -> TaskID final;
    /** Start up a new client session
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    auto StartClientSession(const opentxs::Options& args, const int instance)
        const -> const api::session::Client&;
    auto StartClientSession(const int instance) const
        -> const api::session::Client&;
    auto StartClientSession(
        const opentxs::Options& args,
        const int instance,
        std::string_view recoverWords,
        std::string_view recoverPassphrase) const
        -> const api::session::Client&;
    /** Start up a new server session
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    auto StartNotarySession(const opentxs::Options& args, const int instance)
        const -> const session::Notary&;
    auto StartNotarySession(const int instance) const -> const session::Notary&;
    /** Access ZAP configuration API */
    auto ZAP() const noexcept -> const api::network::ZAP&;
    /// Returns a handle to the top-level ZMQ API.
    auto ZMQ() const noexcept -> const opentxs::network::zeromq::Context&;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Context&;

    OPENTXS_NO_EXPORT Context(internal::Context* imp) noexcept;
    Context() = delete;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;

    OPENTXS_NO_EXPORT ~Context() final;

private:
    internal::Context* imp_;
};
