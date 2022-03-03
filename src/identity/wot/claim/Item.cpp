// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "opentxs/identity/wot/claim/Item.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/credential/Contact.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/ContactItem.pb.h"

namespace opentxs::identity::wot::claim
{
static auto extract_attributes(const proto::ContactItem& serialized)
    -> UnallocatedSet<claim::Attribute>
{
    UnallocatedSet<claim::Attribute> output{};

    for (const auto& attribute : serialized.attribute()) {
        output.emplace(static_cast<claim::Attribute>(attribute));
    }

    return output;
}

static auto extract_attributes(const Claim& claim)
    -> UnallocatedSet<claim::Attribute>
{
    UnallocatedSet<claim::Attribute> output{};

    for (const auto& attribute : std::get<6>(claim)) {
        output.emplace(static_cast<claim::Attribute>(attribute));
    }

    return output;
}

struct Item::Imp {
    const api::Session& api_;
    const VersionNumber version_;
    const UnallocatedCString nym_;
    const claim::SectionType section_;
    const claim::ClaimType type_;
    const UnallocatedCString value_;
    const std::time_t start_;
    const std::time_t end_;
    const UnallocatedSet<claim::Attribute> attributes_;
    const OTIdentifier id_;
    const UnallocatedCString subtype_;

    static auto check_version(
        const VersionNumber in,
        const VersionNumber targetVersion) -> VersionNumber
    {
        // Upgrade version
        if (targetVersion > in) { return targetVersion; }

        return in;
    }

    Imp(const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const claim::ClaimType& type,
        const UnallocatedCString& value,
        const UnallocatedSet<claim::Attribute>& attributes,
        const std::time_t start,
        const std::time_t end,
        const UnallocatedCString subtype)
        : api_(api)
        , version_(check_version(version, parentVersion))
        , nym_(nym)
        , section_(section)
        , type_(type)
        , value_(value)
        , start_(start)
        , end_(end)
        , attributes_(attributes)
        , id_(Identifier::Factory(
              identity::credential::Contact::
                  ClaimID(api, nym, section, type, start, end, value, subtype)))
        , subtype_(subtype)
    {
        if (0 == version) {
            LogError()(OT_PRETTY_CLASS())("Warning: malformed version. "
                                          "Setting to ")(parentVersion)(".")
                .Flush();
        }
    }

    Imp(const Imp& rhs)
        : api_(rhs.api_)
        , version_(rhs.version_)
        , nym_(rhs.nym_)
        , section_(rhs.section_)
        , type_(rhs.type_)
        , value_(rhs.value_)
        , start_(rhs.start_)
        , end_(rhs.end_)
        , attributes_(rhs.attributes_)
        , id_(rhs.id_)
        , subtype_(rhs.subtype_)
    {
    }

    Imp(Imp&& rhs)
        : api_(rhs.api_)
        , version_(rhs.version_)
        , nym_(std::move(const_cast<UnallocatedCString&>(rhs.nym_)))
        , section_(rhs.section_)
        , type_(rhs.type_)
        , value_(std::move(const_cast<UnallocatedCString&>(rhs.value_)))
        , start_(rhs.start_)
        , end_(rhs.end_)
        , attributes_(std::move(
              const_cast<UnallocatedSet<claim::Attribute>&>(rhs.attributes_)))
        , id_(std::move(const_cast<OTIdentifier&>(rhs.id_)))
        , subtype_(std::move(const_cast<UnallocatedCString&>(rhs.subtype_)))
    {
    }

    auto set_attribute(const claim::Attribute& attribute, const bool value)
        const -> Item
    {
        auto attributes = attributes_;

        if (value) {
            attributes.emplace(attribute);

            if (claim::Attribute::Primary == attribute) {
                attributes.emplace(claim::Attribute::Active);
            }
        } else {
            attributes.erase(attribute);
        }

        return Item(
            api_,
            nym_,
            version_,
            version_,
            section_,
            type_,
            value_,
            attributes,
            start_,
            end_,
            subtype_);
    }
};

Item::Item(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const claim::SectionType section,
    const claim::ClaimType& type,
    const UnallocatedCString& value,
    const UnallocatedSet<claim::Attribute>& attributes,
    const std::time_t start,
    const std::time_t end,
    const UnallocatedCString subtype)
    : imp_(std::make_unique<Imp>(
          api,
          nym,
          version,
          parentVersion,
          section,
          type,
          value,
          attributes,
          start,
          end,
          subtype))
{
    OT_ASSERT(imp_);
}

Item::Item(const Item& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    OT_ASSERT(imp_);
}

Item::Item(Item&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

Item::Item(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const Claim& claim)
    : Item(
          api,
          nym,
          version,
          parentVersion,
          static_cast<claim::SectionType>(std::get<1>(claim)),
          static_cast<claim::ClaimType>(std::get<2>(claim)),
          std::get<3>(claim),
          extract_attributes(claim),
          std::get<4>(claim),
          std::get<5>(claim),
          "")
{
}

Item::Item(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber parentVersion,
    const claim::SectionType section,
    const proto::ContactItem& data)
    : Item(
          api,
          nym,
          data.version(),
          parentVersion,
          section,
          translate(data.type()),
          data.value(),
          extract_attributes(data),
          data.start(),
          data.end(),
          data.subtype())
{
}

Item::Item(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber parentVersion,
    const claim::SectionType section,
    const ReadView& bytes)
    : Item(
          api,
          nym,
          parentVersion,
          section,
          proto::Factory<proto::ContactItem>(bytes))
{
}

auto Item::operator==(const Item& rhs) const -> bool
{
    if (false == (imp_->version_ == rhs.imp_->version_)) { return false; }

    if (false == (imp_->nym_ == rhs.imp_->nym_)) { return false; }

    if (false == (imp_->section_ == rhs.imp_->section_)) { return false; }

    if (false == (imp_->type_ == rhs.imp_->type_)) { return false; }

    if (false == (imp_->value_ == rhs.imp_->value_)) { return false; }

    if (false == (imp_->start_ == rhs.imp_->start_)) { return false; }

    if (false == (imp_->end_ == rhs.imp_->end_)) { return false; }

    if (false == (imp_->attributes_ == rhs.imp_->attributes_)) { return false; }

    if (false == (imp_->id_ == rhs.imp_->id_)) { return false; }

    return true;
}

auto Item::End() const -> const std::time_t& { return imp_->end_; }

auto Item::ID() const -> const Identifier& { return imp_->id_; }

auto Item::isActive() const -> bool
{
    return 1 == imp_->attributes_.count(claim::Attribute::Active);
}

auto Item::isLocal() const -> bool
{
    return 1 == imp_->attributes_.count(claim::Attribute::Local);
}

auto Item::isPrimary() const -> bool
{
    return 1 == imp_->attributes_.count(claim::Attribute::Primary);
}

auto Item::Section() const -> const claim::SectionType&
{
    return imp_->section_;
}

auto Item::Serialize(AllocateOutput destination, const bool withID) const
    -> bool
{
    return write(
        [&] {
            auto proto = proto::ContactItem{};
            Serialize(proto, withID);

            return proto;
        }(),
        destination);
}

auto Item::Serialize(proto::ContactItem& output, const bool withID) const
    -> bool
{
    output.set_version(imp_->version_);

    if (withID) { output.set_id(String::Factory(imp_->id_)->Get()); }

    output.set_type(translate(imp_->type_));
    output.set_value(imp_->value_);
    output.set_start(imp_->start_);
    output.set_end(imp_->end_);

    for (const auto& attribute : imp_->attributes_) {
        output.add_attribute(translate(attribute));
    }

    return true;
}

auto Item::SetActive(const bool active) const -> Item
{
    const bool existingValue =
        1 == imp_->attributes_.count(claim::Attribute::Active);

    if (existingValue == active) { return *this; }

    return imp_->set_attribute(claim::Attribute::Active, active);
}

auto Item::SetEnd(const std::time_t end) const -> Item
{
    if (imp_->end_ == end) { return *this; }

    return Item(
        imp_->api_,
        imp_->nym_,
        imp_->version_,
        imp_->version_,
        imp_->section_,
        imp_->type_,
        imp_->value_,
        imp_->attributes_,
        imp_->start_,
        end,
        imp_->subtype_);
}

auto Item::SetLocal(const bool local) const -> Item
{
    const bool existingValue =
        1 == imp_->attributes_.count(claim::Attribute::Local);

    if (existingValue == local) { return *this; }

    return imp_->set_attribute(claim::Attribute::Local, local);
}

auto Item::SetPrimary(const bool primary) const -> Item
{
    const bool existingValue =
        1 == imp_->attributes_.count(claim::Attribute::Primary);

    if (existingValue == primary) { return *this; }

    return imp_->set_attribute(claim::Attribute::Primary, primary);
}

auto Item::SetStart(const std::time_t start) const -> Item
{
    if (imp_->start_ == start) { return *this; }

    return Item(
        imp_->api_,
        imp_->nym_,
        imp_->version_,
        imp_->version_,
        imp_->section_,
        imp_->type_,
        imp_->value_,
        imp_->attributes_,
        start,
        imp_->end_,
        imp_->subtype_);
}

auto Item::SetValue(const UnallocatedCString& value) const -> Item
{
    if (imp_->value_ == value) { return *this; }

    return Item(
        imp_->api_,
        imp_->nym_,
        imp_->version_,
        imp_->version_,
        imp_->section_,
        imp_->type_,
        value,
        imp_->attributes_,
        imp_->start_,
        imp_->end_,
        imp_->subtype_);
}

auto Item::Start() const -> const std::time_t& { return imp_->start_; }

auto Item::Subtype() const -> const UnallocatedCString&
{
    return imp_->subtype_;
}

auto Item::Type() const -> const claim::ClaimType& { return imp_->type_; }

auto Item::Value() const -> const UnallocatedCString& { return imp_->value_; }

auto Item::Version() const -> VersionNumber { return imp_->version_; }

Item::~Item() = default;
}  // namespace opentxs::identity::wot::claim
