// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace imp
{
class Session;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api

class Factory;
class PasswordPrompt;

using OTPasswordPrompt = Pimpl<PasswordPrompt>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
/*
 PasswordPrompt
 This class is used for passing user data to the password callback.
 Whenever actually doing some OpenSSL call that involves a private key,
 just instantiate one of these and pass its address as the userdata for
 the OpenSSL call.  Then when the OT password callback is activated by
 OpenSSL, that pointer will be passed into the callback, so the user string
 can be displayed on the password dialog. (And also so the callback knows
 whether it was activated for a normal key or for a master key.) If it was
 activated for a normal key, then it will use the cached master key, or
 if that's timed out then it will try to decrypt a copy of it using the
 master Nym. Whereas if it WAS activated for the Master Nym, then it will
 just pop up the passphrase dialog and get his passphrase, and use that to
 decrypt the master key.

 NOTE: For internationalization later, we can add an PasswordPrompt constructor
 that takes a STRING CODE instead of an actual string. We can use an enum for
 this. Then we just pass the code there, instead of the string itself, and
 the class will do the work of looking up the actual string based on that code.
 */
class OPENTXS_EXPORT PasswordPrompt
{
public:
    auto GetDisplayString() const -> const char*;
    auto Password() const -> const Secret&;

    auto ClearPassword() -> bool;
    auto SetPassword(const Secret& password) -> bool;

    PasswordPrompt(const PasswordPrompt&&) = delete;
    auto operator=(const PasswordPrompt&) -> const PasswordPrompt& = delete;
    auto operator=(const PasswordPrompt&&) -> const PasswordPrompt& = delete;

    ~PasswordPrompt();

private:
    friend OTPasswordPrompt;
    friend opentxs::Factory;
    friend api::session::imp::Session;

    auto clone() const noexcept -> PasswordPrompt*
    {
        return new PasswordPrompt(*this);
    }

    const api::Session& api_;
    UnallocatedCString display_;
    OTSecret password_;

    PasswordPrompt(
        const api::Session& api,
        const UnallocatedCString& display) noexcept;
    PasswordPrompt(const PasswordPrompt&) noexcept;
};
}  // namespace opentxs
