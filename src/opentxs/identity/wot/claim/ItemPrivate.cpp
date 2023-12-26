// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/ItemPrivate.hpp"  // IWYU pragma: associated

#include <ContactItem.pb.h>
#include <utility>

#include "internal/core/identifier/Identifier.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/wot/Claim.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::identity::wot::claim
{
ItemPrivate::ItemPrivate(Claim claim, allocator_type alloc) noexcept
    : internal::Item(alloc)
    , claim_(std::move(claim), alloc)
{
}

ItemPrivate::ItemPrivate(const ItemPrivate& rhs, allocator_type alloc) noexcept
    : internal::Item(rhs, alloc)
    , claim_(rhs.claim_, alloc)
{
}

auto ItemPrivate::CreateModified(
    std::optional<std::string_view> value,
    std::optional<ReadView> subtype,
    std::optional<Time> start,
    std::optional<Time> end,
    allocator_type alloc) const noexcept -> ItemPrivate*
{
    return pmr::construct<ItemPrivate>(
        alloc,
        claim_.CreateModified(
            std::move(value),
            std::move(subtype),
            std::move(start),
            std::move(end),
            alloc));
}

auto ItemPrivate::Add(claim::Attribute value) noexcept -> void
{
    claim_.Add(value);
}

auto ItemPrivate::End() const noexcept -> Time { return claim_.Stop(); }

auto ItemPrivate::for_each_attribute(
    std::function<void(claim::Attribute)> f) const noexcept -> void
{
    claim_.for_each_attribute(std::move(f));
}

auto ItemPrivate::HasAttribute(claim::Attribute value) const noexcept -> bool
{
    return claim_.HasAttribute(value);
}

auto ItemPrivate::ID() const noexcept -> const identifier::Generic&
{
    return claim_.ID();
}

auto ItemPrivate::Remove(claim::Attribute value) noexcept -> void
{
    claim_.Remove(value);
}

auto ItemPrivate::Section() const noexcept -> claim::SectionType
{
    return claim_.Section();
}

auto ItemPrivate::Serialize(Writer&& out, bool withID) const noexcept -> bool
{
    return write(
        [&] {
            auto proto = proto::ContactItem{};
            Serialize(proto, withID);

            return proto;
        }(),
        std::move(out));
}

auto ItemPrivate::Serialize(proto::ContactItem& out, bool withID) const noexcept
    -> bool
{
    out.set_version(Version());

    if (withID && !ID().Internal().Serialize(*out.mutable_id())) {

        return false;
    }

    out.set_type(translate(Type()));
    out.set_value(UnallocatedCString{Value()});
    out.set_start(seconds_since_epoch(Start()).value());
    out.set_end(seconds_since_epoch(End()).value());
    for_each_attribute([&out](auto a) { out.add_attribute(translate(a)); });

    return true;
}

auto ItemPrivate::SetVersion(VersionNumber value) noexcept -> void
{
    claim_.SetVersion(value);
}

auto ItemPrivate::Start() const noexcept -> Time { return claim_.Start(); }

auto ItemPrivate::Subtype() const noexcept -> ReadView
{
    return claim_.Subtype();
}

auto ItemPrivate::Type() const noexcept -> claim::ClaimType
{
    return claim_.Type();
}

auto ItemPrivate::Value() const noexcept -> std::string_view
{
    return claim_.Value();
}

auto ItemPrivate::Version() const noexcept -> VersionNumber
{
    return claim_.Version();
}

ItemPrivate::~ItemPrivate() = default;
}  // namespace opentxs::identity::wot::claim
