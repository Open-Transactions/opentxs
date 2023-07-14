// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/core/contract/peer/Request.hpp"

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

namespace identity
{
namespace wot
{
class Claim;
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class OPENTXS_EXPORT Verification : public Request
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Verification&;

    [[nodiscard]] auto Claim() const noexcept -> const identity::wot::Claim&;
    [[nodiscard]] auto IsValid() const noexcept -> bool override;

    OPENTXS_NO_EXPORT Verification(RequestPrivate* imp) noexcept;
    Verification(allocator_type alloc = {}) noexcept;
    Verification(const Verification& rhs, allocator_type alloc = {}) noexcept;
    Verification(Verification&& rhs) noexcept;
    Verification(Verification&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Verification& rhs) noexcept -> Verification&;
    auto operator=(Verification&& rhs) noexcept -> Verification&;

    ~Verification() override;
};
}  // namespace opentxs::contract::peer::request
