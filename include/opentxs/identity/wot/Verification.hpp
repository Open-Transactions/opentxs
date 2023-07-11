// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <variant>

#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <span>

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/verification/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace wot
{
namespace internal
{
class Verification;
}  // namespace internal

class Verification;
class VerificationPrivate;
}  // namespace wot
}  // namespace identity

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identity::wot::Verification> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identity::wot::Verification&) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::identity::wot::Verification> {
    auto operator()(
        const opentxs::identity::wot::Verification& lhs,
        const opentxs::identity::wot::Verification& rhs) const noexcept -> bool;
};
}  // namespace std

namespace opentxs::identity::wot
{
OPENTXS_EXPORT auto operator==(
    const Verification& lhs,
    const Verification& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(
    const Verification& lhs,
    const Verification& rhs) noexcept -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Verification& lhs, Verification& rhs) noexcept -> void;
}  // namespace opentxs::identity::wot

namespace opentxs::identity::wot
{
class Verification : virtual public Allocated
{
public:
    using identifier_type = VerificationID;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }

    [[nodiscard]] auto Claim() const noexcept -> const ClaimID&;
    [[nodiscard]] auto get_allocator() const noexcept -> allocator_type final;
    [[nodiscard]] auto ID() const noexcept -> const identifier_type&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Verification&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool;
    [[nodiscard]] auto Start() const noexcept -> Time;
    [[nodiscard]] auto Stop() const noexcept -> Time;
    [[nodiscard]] virtual auto Superscedes() const noexcept
        -> std::span<const VerificationID>;
    [[nodiscard]] auto Value() const noexcept -> verification::Type;
    [[nodiscard]] auto Version() const noexcept -> VersionNumber;

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Verification&;
    auto swap(Verification& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Verification(VerificationPrivate* imp) noexcept;
    Verification(allocator_type alloc = {}) noexcept;
    Verification(const Verification& rhs, allocator_type alloc = {}) noexcept;
    Verification(Verification&& rhs) noexcept;
    Verification(Verification&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Verification& rhs) noexcept -> Verification&;
    auto operator=(Verification&& rhs) noexcept -> Verification&;

    ~Verification() override;

protected:
    VerificationPrivate* imp_;
};
}  // namespace opentxs::identity::wot
