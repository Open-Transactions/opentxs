// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/core/contract/peer/Reply.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace contract
{
namespace peer
{
class ReplyPrivate;
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply
{
class OPENTXS_EXPORT StoreSecret : public Reply
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> StoreSecret&;

    [[nodiscard]] auto IsValid() const noexcept -> bool override;
    [[nodiscard]] auto Value() const noexcept -> bool;

    OPENTXS_NO_EXPORT StoreSecret(ReplyPrivate* imp) noexcept;
    StoreSecret(allocator_type alloc = {}) noexcept;
    StoreSecret(const StoreSecret& rhs, allocator_type alloc = {}) noexcept;
    StoreSecret(StoreSecret&& rhs) noexcept;
    StoreSecret(StoreSecret&& rhs, allocator_type alloc) noexcept;
    auto operator=(const StoreSecret& rhs) noexcept -> StoreSecret&;
    auto operator=(StoreSecret&& rhs) noexcept -> StoreSecret&;

    ~StoreSecret() override;
};
}  // namespace opentxs::contract::peer::reply
