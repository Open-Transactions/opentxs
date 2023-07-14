// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "internal/identity/wot/Verification.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot
{
class VerificationPrivate : virtual public internal::Verification,
                            public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> VerificationPrivate*
    {
        return default_construct<VerificationPrivate>(
            alloc::PMR<VerificationPrivate>{alloc});
    }

    [[nodiscard]] virtual auto Claim() const noexcept -> const ClaimID&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> VerificationPrivate*
    {
        return pmr::clone(this, alloc::PMR<VerificationPrivate>{alloc});
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return make_deleter(this);
    }
    [[nodiscard]] virtual auto ID() const noexcept -> const VerificationID&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] virtual auto Serialize(Writer&& out) const noexcept -> bool;
    [[nodiscard]] virtual auto Start() const noexcept -> Time;
    [[nodiscard]] virtual auto Stop() const noexcept -> Time;
    [[nodiscard]] virtual auto Superscedes() const noexcept
        -> std::span<const VerificationID>;
    [[nodiscard]] virtual auto Value() const noexcept -> verification::Type;
    [[nodiscard]] virtual auto Version() const noexcept -> VersionNumber;

    VerificationPrivate(allocator_type alloc) noexcept;
    VerificationPrivate() = delete;
    VerificationPrivate(
        const VerificationPrivate& rhs,
        allocator_type alloc) noexcept;
    VerificationPrivate(const VerificationPrivate&) = delete;
    VerificationPrivate(VerificationPrivate&&) = delete;
    auto operator=(const VerificationPrivate&) -> VerificationPrivate& = delete;
    auto operator=(VerificationPrivate&&) -> VerificationPrivate& = delete;

    ~VerificationPrivate() override = default;
};
}  // namespace opentxs::identity::wot
