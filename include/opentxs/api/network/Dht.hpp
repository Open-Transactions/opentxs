// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
namespace internal
{
class Dht;
}  // namespace internal
}  // namespace network
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
class OPENTXS_EXPORT Dht
{
public:
    virtual auto GetPublicNym(const UnallocatedCString& key) const noexcept
        -> void = 0;
    virtual auto GetServerContract(const UnallocatedCString& key) const noexcept
        -> void = 0;
    virtual auto GetUnitDefinition(const UnallocatedCString& key) const noexcept
        -> void = 0;
    virtual auto Insert(
        const UnallocatedCString& key,
        const UnallocatedCString& value) const noexcept -> void = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Dht& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Dht& = 0;

    OPENTXS_NO_EXPORT virtual ~Dht() = default;

protected:
    Dht() = default;

private:
    Dht(const Dht&) = delete;
    Dht(Dht&&) = delete;
    auto operator=(const Dht&) -> Dht& = delete;
    auto operator=(Dht&&) -> Dht& = delete;
};
}  // namespace opentxs::api::network
