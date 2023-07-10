// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/wot/claim/claim/Implementation.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/identity/credential/Contact.hpp"

namespace opentxs::identity::wot::claim::implementation
{
Claim::Claim(
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
    allocator_type alloc) noexcept
    : ClaimPrivate(alloc)
    , api_(api)
    , version_(version)
    , claimant_(std::move(claimant), alloc)
    , section_(section)
    , type_(type)
    , value_(value, alloc)
    , subtype_(subtype, alloc)
    , start_(start)
    , stop_(stop)
    , preimage_([&] {
        if (preimage.has_value()) {

            return *preimage;
        } else {
            auto out = decltype(preimage_){};
            out.set_version(preimage_version_);
            out.set_nymid(claimant_.asBase58(api_.Crypto()));
            out.set_section(translate(section_));
            out.set_type(translate(type_));
            out.set_start(Clock::to_time_t(start_));
            out.set_end(Clock::to_time_t(stop_));
            out.set_value(
                static_cast<const char*>(value_.data()), value_.size());
            out.set_subtype(
                static_cast<const char*>(subtype_.data()), subtype_.size());

            return out;
        }
    }())
    , id_(credential::Contact::ClaimID(api_, preimage_), alloc)
    , attributes_(std::move(attributes), alloc)
{
}

Claim::Claim(
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
    allocator_type alloc) noexcept
    : Claim(
          api,
          default_version_,
          std::move(claimant),
          section,
          type,
          value,
          subtype,
          start,
          stop,
          std::move(attributes),
          std::move(preimage),
          alloc)
{
}

Claim::Claim(const Claim& rhs, allocator_type alloc) noexcept
    : ClaimPrivate(rhs, alloc)
    , api_(rhs.api_)
    , version_(rhs.version_)
    , claimant_(rhs.claimant_, alloc)
    , section_(rhs.section_)
    , type_(rhs.type_)
    , value_(rhs.value_, alloc)
    , subtype_(rhs.subtype_, alloc)
    , start_(rhs.start_)
    , stop_(rhs.stop_)
    , preimage_(rhs.preimage_)
    , id_(rhs.id_, alloc)
    , attributes_(*rhs.attributes_.lock_shared(), alloc)
{
}

auto Claim::Add(claim::Attribute attr) noexcept -> void
{
    attributes_.lock()->insert(attr);
}

auto Claim::Attributes() const noexcept -> UnallocatedSet<claim::Attribute>
{
    auto out = UnallocatedSet<claim::Attribute>{};
    auto handle = attributes_.lock_shared();
    std::copy(handle->begin(), handle->end(), std::inserter(out, out.end()));

    return out;
}

auto Claim::Attributes(alloc::Strategy alloc) const noexcept
    -> Set<claim::Attribute>
{
    return {*attributes_.lock_shared(), alloc.result_};
}

auto Claim::ChangeValue(ReadView value) noexcept -> wot::Claim
{
    auto alloc = get_allocator();

    return std::make_unique<Claim>(
               api_,
               version_,
               claimant_,
               section_,
               type_,
               value,
               subtype_.Bytes(),
               start_,
               stop_,
               Attributes(alloc),
               std::nullopt,
               alloc)
        .release();
}

auto Claim::Remove(claim::Attribute attr) noexcept -> void
{
    attributes_.lock()->erase(attr);
}

auto Claim::Serialize(Writer&& out) const noexcept -> bool
{
    return proto::write(preimage_, std::move(out));
}
}  // namespace opentxs::identity::wot::claim::implementation
