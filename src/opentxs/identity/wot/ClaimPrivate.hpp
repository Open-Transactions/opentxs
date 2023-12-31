// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <cs_shared_guarded.h>
#include <opentxs/protobuf/Claim.pb.h>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string_view>

#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/Claim.internal.hpp"
#include "opentxs/identity/wot/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identity
{
namespace wot
{
class ClaimPrivate;  // IWYU pragma: keep
}  // namespace wot
}  // namespace identity

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::identity::wot::ClaimPrivate final : public internal::Claim
{
public:
    [[nodiscard]] auto Claimant() const noexcept -> const wot::Claimant& final
    {
        return claimant_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> ClaimPrivate* final
    {
        return pmr::clone(this, alloc::PMR<ClaimPrivate>{alloc});
    }
    [[nodiscard]] auto CreateModified(
        std::optional<std::string_view> value,
        std::optional<ReadView> subtype,
        std::optional<Time> start,
        std::optional<Time> end,
        allocator_type) const noexcept -> wot::Claim final;
    auto for_each_attribute(
        std::function<void(claim::Attribute)>) const noexcept -> void final;
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    [[nodiscard]] auto HasAttribute(claim::Attribute) const noexcept
        -> bool final;
    [[nodiscard]] auto ID() const noexcept
        -> const wot::Claim::identifier_type& final
    {
        return id_;
    }
    [[nodiscard]] auto IsValid() const noexcept -> bool final { return true; }
    [[nodiscard]] auto Section() const noexcept -> claim::SectionType final
    {
        return section_;
    }
    [[nodiscard]] auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Serialize(protobuf::Claim& out) const noexcept -> void final;
    [[nodiscard]] auto Start() const noexcept -> Time final { return start_; }
    [[nodiscard]] auto Stop() const noexcept -> Time final { return stop_; }
    [[nodiscard]] auto Subtype() const noexcept -> ReadView final
    {
        return subtype_.Bytes();
    }
    [[nodiscard]] auto Type() const noexcept -> claim::ClaimType final
    {
        return type_;
    }
    [[nodiscard]] auto Value() const noexcept -> ReadView final
    {
        return value_.Bytes();
    }
    [[nodiscard]] auto Version() const noexcept -> VersionNumber final
    {
        return data_.lock_shared()->version_;
    }

    auto Add(claim::Attribute) noexcept -> void final;
    auto Remove(claim::Attribute) noexcept -> void final;
    auto SetVersion(VersionNumber) noexcept -> void final;

    ClaimPrivate(
        const api::Session& api,
        const wot::Claimant& claimant,
        VersionNumber version,
        claim::SectionType section,
        claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        Time start,
        Time stop,
        std::span<const claim::Attribute> attributes,
        std::optional<protobuf::Claim> preimage,
        allocator_type alloc) noexcept;
    ClaimPrivate() = delete;
    ClaimPrivate(const ClaimPrivate& rhs, allocator_type alloc) noexcept;
    ClaimPrivate(const ClaimPrivate&) = delete;
    ClaimPrivate(ClaimPrivate&&) = delete;
    auto operator=(const ClaimPrivate&) -> ClaimPrivate& = delete;
    auto operator=(ClaimPrivate&&) -> ClaimPrivate& = delete;

    ~ClaimPrivate() final = default;

private:
    struct Data final : public opentxs::pmr::Allocated {
        using Attributes = boost::container::flat_set<
            claim::Attribute,
            std::less<>,
            alloc::PMR<claim::Attribute>>;

        Attributes attributes_;
        VersionNumber version_;

        [[nodiscard]] auto get_deleter() noexcept -> delete_function final
        {
            return pmr::make_deleter(this);
        }

        Data(
            std::span<const claim::Attribute> attributes,
            VersionNumber version,
            allocator_type alloc) noexcept;
        Data(const Data& rhs, allocator_type alloc = {}) noexcept;
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;
    };

    using Guarded = libguarded::shared_guarded<Data, std::shared_mutex>;

    const api::Session& api_;
    const wot::Claimant claimant_;
    const claim::SectionType section_;
    const claim::ClaimType type_;
    const ByteArray value_;
    const ByteArray subtype_;
    const Time start_;
    const Time stop_;
    const protobuf::Claim preimage_;
    const wot::Claim::identifier_type id_;
    Guarded data_;
};
