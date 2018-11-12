// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/contact/ContactItem.hpp"

#include "opentxs/core/crypto/ContactCredential.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#define OT_METHOD "opentxs::ContactItem::"

namespace opentxs
{
ContactItem::ContactItem(
    const std::string& nym,
    const std::uint32_t version,
    const std::uint32_t parentVersion,
    const proto::ContactSectionName section,
    const proto::ContactItemType& type,
    const std::string& value,
    const std::set<proto::ContactItemAttribute>& attributes,
    const std::time_t start,
    const std::time_t end)
    : version_(check_version(version, parentVersion))
    , nym_(nym)
    , section_(section)
    , type_(type)
    , value_(value)
    , start_(start)
    , end_(end)
    , attributes_(attributes)
    , id_(Identifier::Factory(
          ContactCredential::ClaimID(nym, section, type, start, end, value)))
{
    if (0 == version) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: malformed version. "
                                           "Setting to ")(parentVersion)(".")
            .Flush();
    }
}

ContactItem::ContactItem(
    const std::string& nym,
    const std::uint32_t version,
    const std::uint32_t parentVersion,
    const Claim& claim)
    : ContactItem(
          nym,
          version,
          parentVersion,
          static_cast<proto::ContactSectionName>(std::get<1>(claim)),
          static_cast<proto::ContactItemType>(std::get<2>(claim)),
          std::get<3>(claim),
          extract_attributes(claim),
          std::get<4>(claim),
          std::get<5>(claim))
{
}

ContactItem::ContactItem(
    const std::string& nym,
    const std::uint32_t parentVersion,
    const proto::ContactSectionName section,
    const proto::ContactItem& data)
    : ContactItem(
          nym,
          data.version(),
          parentVersion,
          section,
          data.type(),
          data.value(),
          extract_attributes(data),
          data.start(),
          data.end())
{
}

bool ContactItem::operator==(const ContactItem& rhs) const
{
    if (false == (version_ == rhs.version_)) { return false; }

    if (false == (nym_ == rhs.nym_)) { return false; }

    if (false == (section_ == rhs.section_)) { return false; }

    if (false == (type_ == rhs.type_)) { return false; }

    if (false == (value_ == rhs.value_)) { return false; }

    if (false == (start_ == rhs.start_)) { return false; }

    if (false == (end_ == rhs.end_)) { return false; }

    if (false == (attributes_ == rhs.attributes_)) { return false; }

    if (false == (id_ == rhs.id_)) { return false; }

    return true;
}

ContactItem::operator proto::ContactItem() const { return Serialize(true); }

std::uint32_t ContactItem::check_version(
    const std::uint32_t in,
    const std::uint32_t targetVersion)
{
    // Upgrade version
    if (targetVersion > in) { return targetVersion; }

    return in;
}

const std::time_t& ContactItem::End() const { return end_; }

std::set<proto::ContactItemAttribute> ContactItem::extract_attributes(
    const proto::ContactItem& serialized)
{
    std::set<proto::ContactItemAttribute> output{};

    for (const auto& attribute : serialized.attribute()) {
        output.emplace(static_cast<proto::ContactItemAttribute>(attribute));
    }

    return output;
}

std::set<proto::ContactItemAttribute> ContactItem::extract_attributes(
    const Claim& claim)
{
    std::set<proto::ContactItemAttribute> output{};

    for (const auto& attribute : std::get<6>(claim)) {
        output.emplace(static_cast<proto::ContactItemAttribute>(attribute));
    }

    return output;
}

const Identifier& ContactItem::ID() const { return id_; }

bool ContactItem::isActive() const
{
    return 1 == attributes_.count(proto::CITEMATTR_ACTIVE);
}

bool ContactItem::isLocal() const
{
    return 1 == attributes_.count(proto::CITEMATTR_LOCAL);
}

bool ContactItem::isPrimary() const
{
    return 1 == attributes_.count(proto::CITEMATTR_PRIMARY);
}

const proto::ContactSectionName& ContactItem::Section() const
{
    return section_;
}

proto::ContactItem ContactItem::Serialize(const bool withID) const
{
    proto::ContactItem output{};
    output.set_version(version_);

    if (withID) { output.set_id(String::Factory(id_)->Get()); }

    output.set_type(type_);
    output.set_value(value_);
    output.set_start(start_);
    output.set_end(end_);

    for (const auto& attribute : attributes_) {
        output.add_attribute(attribute);
    }

    return output;
}

ContactItem ContactItem::set_attribute(
    const proto::ContactItemAttribute& attribute,
    const bool value) const
{
    const bool existingValue = 1 == attributes_.count(attribute);

    if (existingValue == value) { return *this; }

    auto attributes = attributes_;

    if (value) {
        attributes.emplace(attribute);

        if (proto::CITEMATTR_PRIMARY == attribute) {
            attributes.emplace(proto::CITEMATTR_ACTIVE);
        }
    } else {
        attributes.erase(attribute);
    }

    return ContactItem(
        nym_,
        version_,
        version_,
        section_,
        type_,
        value_,
        attributes,
        start_,
        end_);
}

ContactItem ContactItem::SetActive(const bool active) const
{
    return set_attribute(proto::CITEMATTR_ACTIVE, active);
}

ContactItem ContactItem::SetEnd(const std::time_t end) const
{
    if (end_ == end) { return *this; }

    return ContactItem(
        nym_,
        version_,
        version_,
        section_,
        type_,
        value_,
        attributes_,
        start_,
        end);
}

ContactItem ContactItem::SetLocal(const bool local) const
{
    return set_attribute(proto::CITEMATTR_LOCAL, local);
}

ContactItem ContactItem::SetPrimary(const bool primary) const
{
    return set_attribute(proto::CITEMATTR_PRIMARY, primary);
}

ContactItem ContactItem::SetStart(const std::time_t start) const
{
    if (start_ == start) { return *this; }

    return ContactItem(
        nym_,
        version_,
        version_,
        section_,
        type_,
        value_,
        attributes_,
        start,
        end_);
}

ContactItem ContactItem::SetValue(const std::string& value) const
{
    if (value_ == value) { return *this; }

    return ContactItem(
        nym_,
        version_,
        version_,
        section_,
        type_,
        value,
        attributes_,
        start_,
        end_);
}

const std::time_t& ContactItem::Start() const { return start_; }

const proto::ContactItemType& ContactItem::Type() const { return type_; }

const std::string& ContactItem::Value() const { return value_; }

const std::uint32_t& ContactItem::Version() const { return version_; }
}  // namespace opentxs
