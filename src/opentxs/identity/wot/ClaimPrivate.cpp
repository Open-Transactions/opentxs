// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/ClaimPrivate.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <opentxs/protobuf/ContactItem.pb.h>
#include <algorithm>
#include <exception>
#include <memory>
#include <utility>

#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::identity::wot
{
ClaimPrivate::ClaimPrivate(
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
    allocator_type alloc) noexcept
    : internal::Claim(alloc)
    , api_(api)
    , claimant_(copy(claimant, {alloc, alloc}))
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
            out.set_version(version);
            get_identifier(claimant_).Internal().Serialize(*out.mutable_nym());
            out.set_section(translate(section_));
            auto& item = *out.mutable_item();
            item.set_version(version);
            item.set_type(translate(type_));
            item.set_start(seconds_since_epoch(start_).value());
            item.set_end(seconds_since_epoch(stop_).value());
            item.set_value(
                static_cast<const char*>(value_.data()), value_.size());
            item.set_subtype(
                static_cast<const char*>(subtype_.data()), subtype_.size());

            return out;
        }
    }())
    , id_(api_.Factory().Internal().IdentifierFromPreimage(preimage_, alloc))
    , data_(attributes, version, alloc)
{
}

ClaimPrivate::ClaimPrivate(
    const ClaimPrivate& rhs,
    allocator_type alloc) noexcept
    : internal::Claim(rhs, alloc)
    , api_(rhs.api_)
    , claimant_(copy(rhs.claimant_, {alloc, alloc}))
    , section_(rhs.section_)
    , type_(rhs.type_)
    , value_(rhs.value_, alloc)
    , subtype_(rhs.subtype_, alloc)
    , start_(rhs.start_)
    , stop_(rhs.stop_)
    , preimage_(rhs.preimage_)
    , id_(rhs.id_, alloc)
    , data_(*rhs.data_.lock_shared(), alloc)
{
}

auto ClaimPrivate::Add(claim::Attribute attr) noexcept -> void
{
    auto handle = data_.lock();
    auto& map = handle->attributes_;
    map.insert(attr);
    using enum claim::Attribute;

    if (Primary == attr) { map.insert(Active); }
}

auto ClaimPrivate::CreateModified(
    std::optional<std::string_view> value,
    std::optional<ReadView> subtype,
    std::optional<Time> start,
    std::optional<Time> end,
    allocator_type alloc) const noexcept -> wot::Claim
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;

    return pmr::construct<ClaimPrivate>(
        alloc,
        api_,
        claimant_,
        data.version_,
        section_,
        type_,
        value.value_or(value_.Bytes()),
        subtype.value_or(subtype_.Bytes()),
        start.value_or(start_),
        end.value_or(stop_),
        data.attributes_,
        std::nullopt);
}

auto ClaimPrivate::for_each_attribute(
    std::function<void(claim::Attribute)> f) const noexcept -> void
{
    const auto handle = data_.lock_shared();
    const auto& data = *handle;

    try {
        std::ranges::for_each(data.attributes_, f);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
    } catch (...) {
        LogError()()("unknown exception").Flush();
    }
}

auto ClaimPrivate::HasAttribute(claim::Attribute value) const noexcept -> bool
{
    return data_.lock_shared()->attributes_.contains(value);
}

auto ClaimPrivate::Remove(claim::Attribute attr) noexcept -> void
{
    auto handle = data_.lock();
    auto& map = handle->attributes_;
    map.erase(attr);
    using enum claim::Attribute;

    if (Active == attr) { map.erase(Primary); }
}

auto ClaimPrivate::Serialize(Writer&& out) const noexcept -> bool
{
    auto proto = preimage_;
    for_each_attribute([&proto](auto a) {
        proto.mutable_item()->add_attribute(translate(a));
    });

    return protobuf::write(preimage_, std::move(out));
}

auto ClaimPrivate::Serialize(protobuf::Claim& out) const noexcept -> void
{
    out.CopyFrom(preimage_);
}

auto ClaimPrivate::SetVersion(VersionNumber value) noexcept -> void
{
    data_.lock()->version_ = value;
}
}  // namespace opentxs::identity::wot
