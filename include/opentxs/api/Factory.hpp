// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/Secret.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace internal
{
class Factory;
}  // namespace internal
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api
{
class OPENTXS_EXPORT Factory
{
public:
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Factory& = 0;
    virtual auto Secret(const std::size_t bytes) const noexcept -> OTSecret = 0;
    virtual auto SecretFromBytes(const ReadView bytes) const noexcept
        -> OTSecret = 0;
    virtual auto SecretFromText(const std::string_view text) const noexcept
        -> OTSecret = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Factory& = 0;

    OPENTXS_NO_EXPORT virtual ~Factory() = default;

protected:
    Factory() noexcept = default;

private:
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;
};
}  // namespace opentxs::api
