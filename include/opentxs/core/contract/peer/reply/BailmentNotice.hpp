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
class OPENTXS_EXPORT BailmentNotice : public Reply
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> BailmentNotice&;

    [[nodiscard]] auto IsValid() const noexcept -> bool override;
    [[nodiscard]] auto Value() const noexcept -> bool;

    OPENTXS_NO_EXPORT BailmentNotice(ReplyPrivate* imp) noexcept;
    BailmentNotice(allocator_type alloc = {}) noexcept;
    BailmentNotice(
        const BailmentNotice& rhs,
        allocator_type alloc = {}) noexcept;
    BailmentNotice(BailmentNotice&& rhs) noexcept;
    BailmentNotice(BailmentNotice&& rhs, allocator_type alloc) noexcept;
    auto operator=(const BailmentNotice& rhs) noexcept -> BailmentNotice&;
    auto operator=(BailmentNotice&& rhs) noexcept -> BailmentNotice&;

    ~BailmentNotice() override;
};
}  // namespace opentxs::contract::peer::reply
