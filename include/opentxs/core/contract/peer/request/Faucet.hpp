// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class OPENTXS_EXPORT Faucet : public Request
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Faucet&;

    [[nodiscard]] auto Currency() const noexcept -> opentxs::UnitType;
    [[nodiscard]] auto Instructions() const noexcept -> std::string_view;
    [[nodiscard]] auto IsValid() const noexcept -> bool override;

    OPENTXS_NO_EXPORT Faucet(RequestPrivate* imp) noexcept;
    Faucet(allocator_type alloc = {}) noexcept;
    Faucet(const Faucet& rhs, allocator_type alloc = {}) noexcept;
    Faucet(Faucet&& rhs) noexcept;
    Faucet(Faucet&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Faucet& rhs) noexcept -> Faucet&;
    auto operator=(Faucet&& rhs) noexcept -> Faucet&;

    ~Faucet() override;
};
}  // namespace opentxs::contract::peer::request
