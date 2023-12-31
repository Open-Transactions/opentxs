// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <optional>
#include <string_view>

#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace wot
{
namespace internal
{
class Claim;  // IWYU pragma: keep
}  // namespace internal
}  // namespace wot
}  // namespace identity

namespace protobuf
{
class Claim;
}  // namespace protobuf

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::identity::wot::internal::Claim : public opentxs::pmr::Allocated
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept -> Claim*
    {
        return pmr::default_construct<Claim>(alloc::PMR<Claim>{alloc});
    }

    [[nodiscard]] virtual auto Claimant() const noexcept
        -> const wot::Claimant&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> Claim*
    {
        return pmr::clone(this, alloc::PMR<Claim>{alloc});
    }
    [[nodiscard]] virtual auto CreateModified(
        std::optional<std::string_view> value = std::nullopt,
        std::optional<ReadView> subtype = std::nullopt,
        std::optional<Time> start = std::nullopt,
        std::optional<Time> end = std::nullopt,
        allocator_type = {}) const noexcept -> wot::Claim;
    virtual auto for_each_attribute(
        std::function<void(claim::Attribute)>) const noexcept -> void;
    [[nodiscard]] virtual auto HasAttribute(claim::Attribute) const noexcept
        -> bool;
    [[nodiscard]] virtual auto ID() const noexcept
        -> const wot::Claim::identifier_type&;
    [[nodiscard]] virtual auto IsValid() const noexcept -> bool;
    [[nodiscard]] virtual auto Section() const noexcept -> claim::SectionType;
    virtual auto Serialize(protobuf::Claim& out) const noexcept -> void;
    [[nodiscard]] virtual auto Serialize(Writer&& out) const noexcept -> bool;
    [[nodiscard]] virtual auto Start() const noexcept -> Time;
    [[nodiscard]] virtual auto Stop() const noexcept -> Time;
    [[nodiscard]] virtual auto Subtype() const noexcept -> ReadView;
    [[nodiscard]] virtual auto Type() const noexcept -> claim::ClaimType;
    [[nodiscard]] virtual auto Value() const noexcept -> ReadView;
    [[nodiscard]] virtual auto Version() const noexcept -> VersionNumber;

    virtual auto Add(claim::Attribute) noexcept -> void;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    virtual auto Remove(claim::Attribute) noexcept -> void;
    virtual auto SetVersion(VersionNumber) noexcept -> void;

    Claim(allocator_type alloc) noexcept;
    Claim() = delete;
    Claim(const Claim& rhs, allocator_type alloc) noexcept;
    Claim(const Claim&) = delete;
    Claim(Claim&&) = delete;
    auto operator=(const Claim&) -> Claim& = delete;
    auto operator=(Claim&&) -> Claim& = delete;

    ~Claim() override = default;
};
