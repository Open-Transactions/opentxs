// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/peer/RequestType.hpp"

#pragma once

#include <optional>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/core/contract/peer/reply/Verification.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class VerificationPrivate : virtual public internal::Verification,
                            virtual public peer::ReplyPrivate
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> VerificationPrivate*
    {
        return pmr::default_construct<VerificationPrivate>(
            alloc::PMR<VerificationPrivate>{alloc});
    }

    [[nodiscard]] virtual auto Accepted() const noexcept -> bool;
    [[nodiscard]] auto asVerificationPrivate() const& noexcept
        -> const reply::VerificationPrivate* final
    {
        return this;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> peer::ReplyPrivate* override
    {
        return pmr::clone(this, alloc::PMR<VerificationPrivate>{alloc});
    }
    [[nodiscard]] virtual auto Response() const noexcept
        -> const std::optional<identity::wot::Verification>&;
    [[nodiscard]] auto Type() const noexcept -> RequestType final;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
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
}  // namespace opentxs::contract::peer::reply
