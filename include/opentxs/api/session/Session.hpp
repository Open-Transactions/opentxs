// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>

#include "opentxs/api/Periodic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class QObject;

namespace opentxs::api
{
class OPENTXS_EXPORT Session : virtual public Periodic
{
public:
    virtual auto Config() const noexcept -> const api::Settings& = 0;
    virtual auto Crypto() const noexcept -> const session::Crypto& = 0;
    virtual auto DataFolder() const noexcept -> const UnallocatedCString& = 0;
    virtual auto Endpoints() const noexcept -> const session::Endpoints& = 0;
    virtual auto Factory() const noexcept -> const session::Factory& = 0;
    virtual auto GetOptions() const noexcept -> const Options& = 0;
    virtual auto Instance() const noexcept -> int = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const session::internal::Session& = 0;
    virtual auto Network() const noexcept -> const network::Network& = 0;
    virtual auto QtRootObject() const noexcept -> QObject* = 0;
    virtual auto SetMasterKeyTimeout(
        const std::chrono::seconds& timeout) const noexcept -> void = 0;
    OPENTXS_NO_EXPORT virtual auto Storage() const noexcept
        -> const session::Storage& = 0;
    virtual auto Wallet() const noexcept -> const session::Wallet& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> session::internal::Session& = 0;

    OPENTXS_NO_EXPORT ~Session() override = default;

protected:
    Session() = default;

private:
    Session(const Session&) = delete;
    Session(Session&&) = delete;
    auto operator=(const Session&) -> Session& = delete;
    auto operator=(Session&&) -> Session& = delete;
};
}  // namespace opentxs::api
