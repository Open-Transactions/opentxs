// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <filesystem>

#include "opentxs/Export.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QObject;

namespace opentxs
{
namespace api
{
namespace internal
{
class Session;
}  // namespace internal

namespace network
{
class Network;
}  // namespace network

namespace session
{
class Crypto;
class Endpoints;
class Factory;
class Storage;
class Wallet;
}  // namespace session

class Session;  // IWYU pragma: keep
class Settings;
}  // namespace api

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 This is the Session API, used for all client and server sessions.
 */
class OPENTXS_EXPORT opentxs::api::Session : virtual public Periodic
{
public:
    /// Cancels a periodic task.
    auto Cancel(TaskID task) const noexcept -> bool final;
    /// Returns a handle to the session-level config API.
    auto Config() const noexcept -> const api::Settings&;
    /// Returns a handle to the session-level crypto API.
    auto Crypto() const noexcept -> const session::Crypto&;
    /// Returns the data folder for this session.
    auto DataFolder() const noexcept -> const std::filesystem::path&;
    /// Returns the Endpoints for this session.
    auto Endpoints() const noexcept -> const session::Endpoints&;
    /// Returns the Factory used for instantiating session objects.
    auto Factory() const noexcept -> const session::Factory&;
    /// Returns an Options object.
    auto GetOptions() const noexcept -> const Options&;
    auto Instance() const noexcept -> int;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Session&;
    /// Returns the network API for this session.
    auto Network() const noexcept -> const network::Network&;
    auto QtRootObject() const noexcept -> QObject*;
    /// Reschedules a periodic task.
    auto Reschedule(TaskID task, std::chrono::seconds interval) const noexcept
        -> bool final;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution.
     *
     * \returns: task identifier which may be used to manage the task
     */
    auto Schedule(std::chrono::seconds interval, opentxs::SimpleCallback task)
        const noexcept -> TaskID final;
    auto Schedule(
        std::chrono::seconds interval,
        opentxs::SimpleCallback task,
        std::chrono::seconds last) const noexcept -> TaskID final;
    /// This timeout determines how long the software will keep a master key
    /// available in memory.
    auto SetMasterKeyTimeout(const std::chrono::seconds& timeout) const noexcept
        -> void;
    auto Storage() const noexcept -> const session::Storage&;
    /// Returns the Wallet API for this session.
    auto Wallet() const noexcept -> const session::Wallet&;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Session&;

    OPENTXS_NO_EXPORT Session(internal::Session* imp) noexcept;
    Session() = delete;
    Session(const Session&) = delete;
    Session(Session&&) = delete;
    auto operator=(const Session&) -> Session& = delete;
    auto operator=(Session&&) -> Session& = delete;

    OPENTXS_NO_EXPORT ~Session() override;

protected:
    friend internal::Session;

    internal::Session* imp_;
};
