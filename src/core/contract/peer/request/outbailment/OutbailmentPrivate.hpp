// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/core/contract/peer/request/Outbailment.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::peer::request
{
class OutbailmentPrivate : virtual public internal::Outbailment,
                           virtual public peer::RequestPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> OutbailmentPrivate*
    {
        return default_construct<OutbailmentPrivate>(
            alloc::PMR<OutbailmentPrivate>{alloc});
    }

    [[nodiscard]] virtual auto Amount() const noexcept -> opentxs::Amount;
    [[nodiscard]] auto asOutbailmentPrivate() const& noexcept
        -> const request::OutbailmentPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::RequestPrivate* override
    {
        return pmr::clone(this, alloc::PMR<OutbailmentPrivate>{alloc});
    }
    [[nodiscard]] virtual auto Instructions() const noexcept
        -> std::string_view;
    [[nodiscard]] virtual auto Notary() const noexcept
        -> const identifier::Notary&;
    [[nodiscard]] auto Type() const noexcept -> RequestType final;
    [[nodiscard]] virtual auto Unit() const noexcept
        -> const identifier::UnitDefinition&;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return make_deleter(this);
    }

    OutbailmentPrivate(allocator_type alloc) noexcept;
    OutbailmentPrivate() = delete;
    OutbailmentPrivate(
        const OutbailmentPrivate& rhs,
        allocator_type alloc) noexcept;
    OutbailmentPrivate(const OutbailmentPrivate&) = delete;
    OutbailmentPrivate(OutbailmentPrivate&&) = delete;
    auto operator=(const OutbailmentPrivate&) -> OutbailmentPrivate& = delete;
    auto operator=(OutbailmentPrivate&&) -> OutbailmentPrivate& = delete;

    ~OutbailmentPrivate() override;
};
}  // namespace opentxs::contract::peer::request
