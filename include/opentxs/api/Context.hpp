// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <functional>

#include "opentxs/Types.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
class Base;
}  // namespace request

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc

class Options;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::api::Context : virtual public Periodic
{
public:
    using ShutdownCallback = std::function<void()>;

    /** NOTE You must call PrepareSignalHandling() prior to initializating the
     * context if you intend to use signal handling */
    static auto PrepareSignalHandling() noexcept -> void;
    static auto SuggestFolder(const UnallocatedCString& app) noexcept
        -> UnallocatedCString;

    virtual auto Asio() const noexcept -> const network::Asio& = 0;
    /** Throws std::out_of_range if the specified session does not exist. */
    virtual auto ClientSession(const int instance) const noexcept(false)
        -> const api::session::Client& = 0;
    virtual auto ClientSessionCount() const noexcept -> std::size_t = 0;
    virtual auto Config(const UnallocatedCString& path) const noexcept
        -> const api::Settings& = 0;
    virtual auto Crypto() const noexcept -> const api::Crypto& = 0;
    virtual auto Factory() const noexcept -> const api::Factory& = 0;
    /** WARNING You must call PrepareSignalHandling() prior to initializating
     * the context if you intend to use this function */
    virtual auto HandleSignals(
        ShutdownCallback* callback = nullptr) const noexcept -> void = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Context& = 0;
    /** Throws std::out_of_range if the specified session does not exist. */
    virtual auto NotarySession(const int instance) const noexcept(false)
        -> const session::Notary& = 0;
    virtual auto NotarySessionCount() const noexcept -> std::size_t = 0;
    virtual auto ProfileId() const noexcept -> UnallocatedCString = 0;
    OPENTXS_NO_EXPORT virtual auto QtRootObject() const noexcept
        -> QObject* = 0;
    virtual auto RPC(const rpc::request::Base& command) const noexcept
        -> std::unique_ptr<rpc::response::Base> = 0;
    virtual auto RPC(const ReadView command, const AllocateOutput response)
        const noexcept -> bool = 0;
    /** Start up a new client session
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    virtual auto StartClientSession(const Options& args, const int instance)
        const -> const api::session::Client& = 0;
    virtual auto StartClientSession(const int instance) const
        -> const api::session::Client& = 0;
    virtual auto StartClientSession(
        const Options& args,
        const int instance,
        const UnallocatedCString& recoverWords,
        const UnallocatedCString& recoverPassphrase) const
        -> const api::session::Client& = 0;
    /** Start up a new server session
     *
     *  If the specified instance exists, it will be returned.
     *
     *  Otherwise the next instance will be created
     */
    virtual auto StartNotarySession(const Options& args, const int instance)
        const -> const session::Notary& = 0;
    virtual auto StartNotarySession(const int instance) const
        -> const session::Notary& = 0;
    /** Access ZAP configuration API */
    virtual auto ZAP() const noexcept -> const api::network::ZAP& = 0;
    virtual auto ZMQ() const noexcept
        -> const opentxs::network::zeromq::Context& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Context& = 0;

    OPENTXS_NO_EXPORT ~Context() override = default;

protected:
    Context() = default;

private:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;
};
