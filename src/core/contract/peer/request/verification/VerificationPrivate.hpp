// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "internal/core/contract/peer/request/Verification.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class VerificationPrivate : virtual public internal::Verification,
                            virtual public peer::RequestPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> VerificationPrivate*
    {
        return default_construct<VerificationPrivate>(
            alloc::PMR<VerificationPrivate>{alloc});
    }

    [[nodiscard]] auto asVerificationPrivate() const& noexcept
        -> const request::VerificationPrivate* final
    {
        return this;
    }
    [[nodiscard]] virtual auto Claim() const noexcept
        -> const identity::wot::Claim&;
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::RequestPrivate* override
    {
        return pmr::clone(this, alloc::PMR<VerificationPrivate>{alloc});
    }
    [[nodiscard]] auto Type() const noexcept -> RequestType final;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return make_deleter(this);
    }

    VerificationPrivate(allocator_type alloc) noexcept;
    VerificationPrivate() = delete;
    VerificationPrivate(
        const VerificationPrivate& rhs,
        allocator_type alloc) noexcept;
    VerificationPrivate(const VerificationPrivate&) = delete;
    VerificationPrivate(VerificationPrivate&&) = delete;
    auto operator=(const VerificationPrivate&) -> VerificationPrivate& = delete;
    auto operator=(VerificationPrivate&&) -> VerificationPrivate& = delete;

    ~VerificationPrivate() override;
};
}  // namespace opentxs::contract::peer::request
