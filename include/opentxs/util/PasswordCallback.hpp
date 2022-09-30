// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT PasswordCallback
{
public:
    // Asks for password once. (For authentication when using nym.)
    virtual auto runOne(
        Secret& output,
        std::string_view prompt,
        std::string_view key) const noexcept -> void = 0;

    // Asks for password twice. (For confirmation when changing password or
    // creating nym.)
    virtual auto runTwo(
        Secret& output,
        std::string_view prompt,
        std::string_view key) const noexcept -> void = 0;

    PasswordCallback() = default;

    virtual ~PasswordCallback() = default;
};
}  // namespace opentxs
