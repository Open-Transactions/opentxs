// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

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
class OPENTXS_EXPORT Outbailment : public Reply
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Outbailment&;

    [[nodiscard]] auto Description() const noexcept -> std::string_view;
    [[nodiscard]] auto IsValid() const noexcept -> bool override;

    OPENTXS_NO_EXPORT Outbailment(ReplyPrivate* imp) noexcept;
    Outbailment(allocator_type alloc = {}) noexcept;
    Outbailment(const Outbailment& rhs, allocator_type alloc = {}) noexcept;
    Outbailment(Outbailment&& rhs) noexcept;
    Outbailment(Outbailment&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Outbailment& rhs) noexcept -> Outbailment&;
    auto operator=(Outbailment&& rhs) noexcept -> Outbailment&;

    ~Outbailment() override;
};
}  // namespace opentxs::contract::peer::reply
