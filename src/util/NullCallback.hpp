// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordCallback.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Factory;
class OTPassword;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto DefaultPassword() noexcept -> const char*;
}  // namespace opentxs

namespace opentxs::implementation
{
class NullCallback final : virtual public PasswordCallback
{
public:
    auto runOne(
        const char* display,
        Secret& output,
        const UnallocatedCString& key) const -> void final;
    auto runTwo(
        const char* display,
        Secret& output,
        const UnallocatedCString& key) const -> void final;

    NullCallback() = default;

    ~NullCallback() final = default;

private:
    friend opentxs::Factory;

    NullCallback(const NullCallback&) = delete;
    NullCallback(NullCallback&&) = delete;
    auto operator=(const NullCallback&) -> NullCallback& = delete;
    auto operator=(NullCallback&&) -> NullCallback& = delete;
};
}  // namespace opentxs::implementation
