// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

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

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class OPENTXS_EXPORT BailmentNotice : public Request
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> BailmentNotice&;

    [[nodiscard]] auto Amount() const noexcept -> opentxs::Amount;
    [[nodiscard]] auto Description() const noexcept -> std::string_view;
    [[nodiscard]] auto InReferenceToRequest() const noexcept
        -> const identifier_type&;
    [[nodiscard]] auto IsValid() const noexcept -> bool override;
    [[nodiscard]] auto Notary() const noexcept -> const identifier::Notary&;
    [[nodiscard]] auto Unit() const noexcept
        -> const identifier::UnitDefinition&;

    OPENTXS_NO_EXPORT BailmentNotice(RequestPrivate* imp) noexcept;
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
}  // namespace opentxs::contract::peer::request
