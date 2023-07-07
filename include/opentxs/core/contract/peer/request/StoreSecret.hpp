// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
class RequestPrivate;
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class OPENTXS_EXPORT StoreSecret : public Request
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> StoreSecret&;

    [[nodiscard]] auto IsValid() const noexcept -> bool override;
    [[nodiscard]] auto Kind() const noexcept -> SecretType;
    [[nodiscard]] auto Values() const noexcept
        -> std::span<const std::string_view>;

    OPENTXS_NO_EXPORT StoreSecret(RequestPrivate* imp) noexcept;
    StoreSecret(allocator_type alloc = {}) noexcept;
    StoreSecret(const StoreSecret& rhs, allocator_type alloc = {}) noexcept;
    StoreSecret(StoreSecret&& rhs) noexcept;
    StoreSecret(StoreSecret&& rhs, allocator_type alloc) noexcept;
    auto operator=(const StoreSecret& rhs) noexcept -> StoreSecret&;
    auto operator=(StoreSecret&& rhs) noexcept -> StoreSecret&;

    ~StoreSecret() override;
};
}  // namespace opentxs::contract::peer::request
