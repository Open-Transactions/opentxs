// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <future>
#include <mutex>

#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
}  // namespace session

class Legacy;
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

class Secret;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/** Callbacks in this form allow OpenSSL to query opentxs to get key encryption
 *  and decryption passwords*/
extern "C" {
using INTERNAL_PASSWORD_CALLBACK =
    std::int32_t(char*, std::int32_t, std::int32_t, void*);
}

namespace opentxs::api::session::internal
{
class Session : virtual public api::Session
{
public:
    virtual auto Contacts() const -> const session::Contacts& = 0;
    virtual auto GetInternalPasswordCallback() const
        -> INTERNAL_PASSWORD_CALLBACK* = 0;
    virtual auto GetSecret(
        const opentxs::Lock& lock,
        Secret& secret,
        const PasswordPrompt& reason,
        const bool twice,
        const UnallocatedCString& key = "") const -> bool = 0;
    // WARNING do not call until the Session is fully constructed
    virtual auto GetShared() const noexcept
        -> std::shared_ptr<const api::Session> = 0;
    auto Internal() const noexcept -> const Session& final { return *this; }
    virtual auto Legacy() const noexcept -> const api::Legacy& = 0;
    virtual auto Lock() const -> std::mutex& = 0;
    virtual auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::symmetric::Key& = 0;
    virtual auto NewNym(const identifier::Nym& id) const noexcept -> void = 0;
    virtual auto ShuttingDown() const noexcept -> bool = 0;

    auto Internal() noexcept -> Session& final { return *this; }
    virtual auto Stop() noexcept -> std::future<void> = 0;

    ~Session() override = default;
};
}  // namespace opentxs::api::session::internal
