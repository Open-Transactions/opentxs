// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Context;
}  // namespace api

class Options;
class PasswordCaller;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
/** Context accessor
 *
 *  Returns a reference to the context
 *
 *  \throws std::runtime_error if the context is not initialized
 */
OPENTXS_EXPORT auto Context() noexcept(false) -> const api::Context&;

/** Shut down context
 *
 *  Call this when the application is closing, after all OT operations
 *  are complete.
 */
OPENTXS_EXPORT auto Cleanup() noexcept -> void;

/** Start up context
 *
 *  Returns a reference to the context singleton after it has been
 *  initialized.
 *
 *  Call this during application startup, before attempting any OT operation
 *
 *  \throws std::runtime_error if the context is already initialized
 */
OPENTXS_EXPORT auto InitContext() noexcept(false) -> const api::Context&;
OPENTXS_EXPORT auto InitContext(const Options& args) noexcept(false)
    -> const api::Context&;
OPENTXS_EXPORT auto InitContext(
    PasswordCaller* externalPasswordCallback) noexcept(false)
    -> const api::Context&;
OPENTXS_EXPORT auto InitContext(
    const Options& args,
    PasswordCaller* externalPasswordCallback) noexcept(false)
    -> const api::Context&;

/** Wait on context shutdown
 *
 *  Blocks until the context has been shut down
 */
OPENTXS_EXPORT auto Join() noexcept -> void;

using LicenseMap = UnallocatedMap<UnallocatedCString, UnallocatedCString>;

OPENTXS_EXPORT auto LicenseData() noexcept -> const LicenseMap&;
OPENTXS_EXPORT auto VersionMajor() noexcept -> unsigned int;
OPENTXS_EXPORT auto VersionMinor() noexcept -> unsigned int;
OPENTXS_EXPORT auto VersionPatch() noexcept -> unsigned int;
OPENTXS_EXPORT auto VersionString() noexcept -> const char*;
}  // namespace opentxs
