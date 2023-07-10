// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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
class OPENTXS_EXPORT Connection : public Request
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Connection&;

    [[nodiscard]] auto IsValid() const noexcept -> bool override;
    [[nodiscard]] auto Kind() const noexcept -> ConnectionInfoType;

    OPENTXS_NO_EXPORT Connection(RequestPrivate* imp) noexcept;
    Connection(allocator_type alloc = {}) noexcept;
    Connection(const Connection& rhs, allocator_type alloc = {}) noexcept;
    Connection(Connection&& rhs) noexcept;
    Connection(Connection&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Connection& rhs) noexcept -> Connection&;
    auto operator=(Connection&& rhs) noexcept -> Connection&;

    ~Connection() override;
};
}  // namespace opentxs::contract::peer::request
