// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/peer/RequestType.hpp"

#pragma once

#include <string_view>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/core/contract/peer/request/Faucet.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
class FaucetPrivate : virtual public internal::Faucet,
                      virtual public peer::RequestPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> FaucetPrivate*
    {
        return pmr::default_construct<FaucetPrivate>(
            alloc::PMR<FaucetPrivate>{alloc});
    }

    [[nodiscard]] auto asFaucetPrivate() const& noexcept
        -> const request::FaucetPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::RequestPrivate* override
    {
        return pmr::clone(this, alloc::PMR<FaucetPrivate>{alloc});
    }
    [[nodiscard]] virtual auto Currency() const noexcept -> opentxs::UnitType;
    [[nodiscard]] virtual auto Instructions() const noexcept
        -> std::string_view;
    [[nodiscard]] auto Type() const noexcept -> RequestType final;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    FaucetPrivate(allocator_type alloc) noexcept;
    FaucetPrivate() = delete;
    FaucetPrivate(const FaucetPrivate& rhs, allocator_type alloc) noexcept;
    FaucetPrivate(const FaucetPrivate&) = delete;
    FaucetPrivate(FaucetPrivate&&) = delete;
    auto operator=(const FaucetPrivate&) -> FaucetPrivate& = delete;
    auto operator=(FaucetPrivate&&) -> FaucetPrivate& = delete;

    ~FaucetPrivate() override;
};
}  // namespace opentxs::contract::peer::request
