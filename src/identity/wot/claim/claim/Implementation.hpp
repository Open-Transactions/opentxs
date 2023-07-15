// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Claim.pb.h>
#include <cs_shared_guarded.h>
#include <functional>
#include <optional>
#include <shared_mutex>

#include "identity/wot/claim/claim/ClaimPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::claim::implementation
{
class Claim final : public ClaimPrivate
{
public:
    [[nodiscard]] auto Attributes() const noexcept
        -> UnallocatedSet<claim::Attribute> final;
    [[nodiscard]] auto Attributes(alloc::Strategy alloc) const noexcept
        -> Set<claim::Attribute> final;
    [[nodiscard]] auto Claimant() const noexcept -> const identifier::Nym& final
    {
        return claimant_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> ClaimPrivate* final
    {
        return pmr::clone(this, alloc::PMR<Claim>{alloc});
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }
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
    auto Serialize(proto::Claim& out) const noexcept -> void final;
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
        return version_;
    }

    auto Add(claim::Attribute) noexcept -> void final;
    [[nodiscard]] auto ChangeValue(ReadView value) noexcept -> wot::Claim final;
    auto Remove(claim::Attribute) noexcept -> void final;

    Claim(
        const api::Session& api,
        identifier::Nym claimant,
        claim::SectionType section,
        claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        Time start,
        Time stop,
        Set<claim::Attribute> attributes,
        std::optional<proto::Claim> preimage,
        allocator_type alloc) noexcept;
    Claim(
        const api::Session& api,
        VersionNumber version,
        identifier::Nym claimant,
        claim::SectionType section,
        claim::ClaimType type,
        ReadView value,
        ReadView subtype,
        Time start,
        Time stop,
        Set<claim::Attribute> attributes,
        std::optional<proto::Claim> preimage,
        allocator_type alloc) noexcept;
    Claim() = delete;
    Claim(const Claim& rhs, allocator_type alloc) noexcept;
    Claim(const Claim&) = delete;
    Claim(Claim&&) = delete;
    auto operator=(const Claim&) -> Claim& = delete;
    auto operator=(Claim&&) -> Claim& = delete;

    ~Claim() final = default;

private:
    using GuardedAttributes =
        libguarded::shared_guarded<Set<claim::Attribute>, std::shared_mutex>;

    static constexpr auto default_version_ = VersionNumber{6u};

    const api::Session& api_;
    const VersionNumber version_;
    const identifier::Nym claimant_;
    const claim::SectionType section_;
    const claim::ClaimType type_;
    const ByteArray value_;
    const ByteArray subtype_;
    const Time start_;
    const Time stop_;
    const proto::Claim preimage_;
    const wot::Claim::identifier_type id_;
    GuardedAttributes attributes_;
};
}  // namespace opentxs::identity::wot::claim::implementation
