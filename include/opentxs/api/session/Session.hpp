// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <filesystem>

#include "opentxs/Export.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Seed;
class Symmetric;
}  // namespace crypto

namespace network
{
class Network;
}  // namespace network

namespace session
{
namespace internal
{
class Session;
}  // namespace internal

class Crypto;
class Endpoints;
class Factory;
class Storage;
class Wallet;
}  // namespace session

class Crypto;
class Settings;
}  // namespace api

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class QObject;

namespace opentxs::api
{
/**
 This is the Session API, used for all client and server sessions.
 */
class OPENTXS_EXPORT Session : virtual public Periodic
{
public:
    /// Returns a handle to the session-level config API.
    virtual auto Config() const noexcept -> const api::Settings& = 0;
    /// Returns a handle to the session-level crypto API.
    virtual auto Crypto() const noexcept -> const session::Crypto& = 0;
    /// Returns the data folder for this session.
    virtual auto DataFolder() const noexcept
        -> const std::filesystem::path& = 0;
    /// Returns the Endpoints for this session.
    virtual auto Endpoints() const noexcept -> const session::Endpoints& = 0;
    /// Returns the Factory used for instantiating session objects.
    virtual auto Factory() const noexcept -> const session::Factory& = 0;
    /// Returns an Options object.
    virtual auto GetOptions() const noexcept -> const Options& = 0;
    virtual auto Instance() const noexcept -> int = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const session::internal::Session& = 0;
    /// Returns the network API for this session.
    virtual auto Network() const noexcept -> const network::Network& = 0;
    virtual auto QtRootObject() const noexcept -> QObject* = 0;
    /// This timeout determines how long the software will keep a master key
    /// available in memory.
    virtual auto SetMasterKeyTimeout(
        const std::chrono::seconds& timeout) const noexcept -> void = 0;
    OPENTXS_NO_EXPORT virtual auto Storage() const noexcept
        -> const session::Storage& = 0;
    /// Returns the Wallet API for this session.
    virtual auto Wallet() const noexcept -> const session::Wallet& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> session::internal::Session& = 0;

    Session(const Session&) = delete;
    Session(Session&&) = delete;
    auto operator=(const Session&) -> Session& = delete;
    auto operator=(Session&&) -> Session& = delete;

    OPENTXS_NO_EXPORT ~Session() override = default;

protected:
    Session() = default;
};
}  // namespace opentxs::api
