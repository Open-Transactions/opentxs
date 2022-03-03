// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
namespace internal
{
class Util;
}  // namespace internal
}  // namespace crypto
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
class Util
{
public:
    OPENTXS_NO_EXPORT virtual auto InternalUtil() const noexcept
        -> const internal::Util& = 0;
    virtual auto RandomizeMemory(void* destination, const std::size_t size)
        const -> bool = 0;

    OPENTXS_NO_EXPORT virtual auto InternalUtil() noexcept
        -> internal::Util& = 0;

    OPENTXS_NO_EXPORT virtual ~Util() = default;

protected:
    Util() = default;

private:
    Util(const Util&) = delete;
    Util(Util&&) = delete;
    auto operator=(const Util&) -> Util& = delete;
    auto operator=(Util&&) -> Util& = delete;
};
}  // namespace opentxs::api::crypto
