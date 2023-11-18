// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>

#include "internal/util/Mutex.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QObject;

namespace opentxs
{
namespace api
{
namespace internal
{
class Paths;
class Session;  // IWYU pragma: keep
}  // namespace internal

namespace network
{
class Network;
}  // namespace network

namespace session
{
namespace internal
{
class Client;
class Notary;
}  // namespace internal

class Client;
class Contacts;
class Crypto;
class Endpoints;
class Factory;
class Notary;
class Storage;
class Wallet;
}  // namespace session

class Session;
class Settings;
}  // namespace api

namespace crypto
{
namespace symmetric
{
class Key;
}  // namespace symmetric
}  // namespace crypto

namespace identifier
{
class Nym;
}  // namespace identifier

class Options;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/** Callbacks in this form allow OpenSSL to query opentxs to get key encryption
 *  and decryption passwords*/
extern "C" {
using INTERNAL_PASSWORD_CALLBACK =
    std::int32_t(char*, std::int32_t, std::int32_t, void*);
}

class opentxs::api::internal::Session : virtual public Periodic
{
public:
    static auto Detach(api::Session& self) noexcept -> void;

    virtual auto asClient() const noexcept
        -> const session::internal::Client& = 0;
    virtual auto asClientPublic() const noexcept -> const session::Client& = 0;
    virtual auto asNotary() const noexcept
        -> const session::internal::Notary& = 0;
    virtual auto asNotaryPublic() const noexcept -> const session::Notary& = 0;
    virtual auto Config() const noexcept -> const api::Settings& = 0;
    virtual auto Contacts() const -> const session::Contacts& = 0;
    virtual auto Crypto() const noexcept -> const session::Crypto& = 0;
    virtual auto DataFolder() const noexcept
        -> const std::filesystem::path& = 0;
    virtual auto Endpoints() const noexcept -> const session::Endpoints& = 0;
    virtual auto Factory() const noexcept -> const session::Factory& = 0;
    virtual auto GetInternalPasswordCallback() const
        -> INTERNAL_PASSWORD_CALLBACK* = 0;
    virtual auto GetOptions() const noexcept -> const Options& = 0;
    virtual auto GetSecret(
        const opentxs::Lock& lock,
        Secret& secret,
        const PasswordPrompt& reason,
        const bool twice,
        const UnallocatedCString& key = "") const -> bool = 0;
    // WARNING do not call until the Session is fully constructed
    virtual auto GetShared() const noexcept
        -> std::shared_ptr<const api::internal::Session> = 0;
    virtual auto Instance() const noexcept -> int = 0;
    virtual auto Lock() const -> std::mutex& = 0;
    virtual auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::symmetric::Key& = 0;
    virtual auto Network() const noexcept -> const network::Network& = 0;
    virtual auto NewNym(const identifier::Nym& id) const noexcept -> void = 0;
    virtual auto Paths() const noexcept -> const api::internal::Paths& = 0;
    virtual auto QtRootObject() const noexcept -> QObject* = 0;
    virtual auto Self() const noexcept -> const api::Session& = 0;
    virtual auto SetMasterKeyTimeout(
        const std::chrono::seconds& timeout) const noexcept -> void = 0;
    virtual auto ShuttingDown() const noexcept -> bool = 0;
    virtual auto Storage() const noexcept -> const session::Storage& = 0;
    virtual auto Wallet() const noexcept -> const session::Wallet& = 0;

    virtual auto asClient() noexcept -> session::internal::Client& = 0;
    virtual auto asClientPublic() noexcept -> session::Client& = 0;
    virtual auto asNotary() noexcept -> session::internal::Notary& = 0;
    virtual auto asNotaryPublic() noexcept -> session::Notary& = 0;
    virtual auto Self() noexcept -> api::Session& = 0;
    virtual auto Stop() noexcept -> std::future<void> = 0;

    ~Session() override = default;
};
