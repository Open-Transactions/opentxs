// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>

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

namespace identity
{
namespace wot
{
class Verification;
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::reply
{
class OPENTXS_EXPORT Verification : public Reply
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Verification&;

    [[nodiscard]] auto Accepted() const noexcept -> bool;
    [[nodiscard]] auto IsValid() const noexcept -> bool override;
    [[nodiscard]] auto Response() const noexcept
        -> const std::optional<identity::wot::Verification>&;

    OPENTXS_NO_EXPORT Verification(ReplyPrivate* imp) noexcept;
    Verification(allocator_type alloc = {}) noexcept;
    Verification(const Verification& rhs, allocator_type alloc = {}) noexcept;
    Verification(Verification&& rhs) noexcept;
    Verification(Verification&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Verification& rhs) noexcept -> Verification&;
    auto operator=(Verification&& rhs) noexcept -> Verification&;

    ~Verification() override;
};
}  // namespace opentxs::contract::peer::reply
