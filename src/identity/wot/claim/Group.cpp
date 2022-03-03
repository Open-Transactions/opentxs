// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "opentxs/identity/wot/claim/Group.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/ContactSection.pb.h"

namespace opentxs::identity::wot::claim
{
struct Group::Imp {
    const UnallocatedCString nym_{};
    const claim::SectionType section_{claim::SectionType::Error};
    const claim::ClaimType type_{claim::ClaimType::Error};
    const OTIdentifier primary_;
    const ItemMap items_{};

    static auto get_primary_item(const ItemMap& items) -> OTIdentifier
    {
        auto primary = Identifier::Factory();

        for (const auto& it : items) {
            const auto& item = it.second;

            OT_ASSERT(item);

            if (item->isPrimary()) {
                primary = item->ID();

                break;
            }
        }

        return primary;
    }

    Imp(const UnallocatedCString& nym,
        const claim::SectionType section,
        const claim::ClaimType type,
        const ItemMap& items)
        : nym_(nym)
        , section_(section)
        , type_(type)
        , primary_(Identifier::Factory(get_primary_item(items)))
        , items_(normalize_items(items))
    {
        for (const auto& it : items_) { OT_ASSERT(it.second); }
    }

    Imp(const Imp& rhs)
        : nym_(rhs.nym_)
        , section_(rhs.section_)
        , type_(rhs.type_)
        , primary_(rhs.primary_)
        , items_(rhs.items_)
    {
    }

    Imp(Imp&& rhs)
        : nym_(std::move(const_cast<UnallocatedCString&>(rhs.nym_)))
        , section_(rhs.section_)
        , type_(rhs.type_)
        , primary_(std::move(const_cast<OTIdentifier&>(rhs.primary_)))
        , items_(std::move(const_cast<ItemMap&>(rhs.items_)))
    {
    }

    static auto normalize_items(const ItemMap& items) -> Group::ItemMap
    {
        auto primary = Identifier::Factory();

        auto map = items;

        for (const auto& it : map) {
            const auto& item = it.second;

            OT_ASSERT(item);

            if (item->isPrimary()) {
                if (primary->empty()) {
                    primary = item->ID();
                } else {
                    const auto& id = item->ID();
                    map[id].reset(new Item(item->SetPrimary(false)));
                }
            }
        }

        return map;
    }
};

static auto create_item(const std::shared_ptr<Item>& item) -> Group::ItemMap
{
    OT_ASSERT(item);

    Group::ItemMap output{};
    output[item->ID()] = item;

    return output;
}

Group::Group(
    const UnallocatedCString& nym,
    const claim::SectionType section,
    const claim::ClaimType type,
    const ItemMap& items)
    : imp_(std::make_unique<Imp>(nym, section, type, items))
{
    OT_ASSERT(imp_);
}

Group::Group(
    const UnallocatedCString& nym,
    const claim::SectionType section,
    const std::shared_ptr<Item>& item)
    : Group(nym, section, item->Type(), create_item(item))
{
    OT_ASSERT(item);
}

Group::Group(const Group& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    OT_ASSERT(imp_);
}

Group::Group(Group&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Group::operator+(const Group& rhs) const -> Group
{
    OT_ASSERT(imp_->section_ == rhs.imp_->section_);

    auto primary = Identifier::Factory();

    if (imp_->primary_->empty()) {
        primary = Identifier::Factory(rhs.imp_->primary_);
    }

    auto map{imp_->items_};

    for (const auto& it : rhs.imp_->items_) {
        const auto& item = it.second;

        OT_ASSERT(item);
        const auto& id = item->ID();
        const bool exists = (1 == map.count(id));

        if (exists) { continue; }

        const bool isPrimary = item->isPrimary();
        const bool designated = (id == primary);

        if (isPrimary && (false == designated)) {
            map.emplace(id, new Item(item->SetPrimary(false)));
        } else {
            map.emplace(id, item);
        }

        OT_ASSERT(map[id])
    }

    return Group(imp_->nym_, imp_->section_, imp_->type_, map);
}

auto Group::AddItem(const std::shared_ptr<Item>& item) const -> Group
{
    OT_ASSERT(item);

    if (item->isPrimary()) { return AddPrimary(item); }

    const auto& id = item->ID();
    const bool alreadyExists =
        (1 == imp_->items_.count(id)) && (*item == *imp_->items_.at(id));

    if (alreadyExists) { return *this; }

    auto map = imp_->items_;
    map[id] = item;

    return Group(imp_->nym_, imp_->section_, imp_->type_, map);
}

auto Group::AddPrimary(const std::shared_ptr<Item>& item) const -> Group
{
    if (false == bool(item)) { return *this; }

    const auto& incomingID = item->ID();
    const bool isExistingPrimary = (imp_->primary_ == incomingID);
    const bool haveExistingPrimary =
        ((false == imp_->primary_->empty()) && (false == isExistingPrimary));
    auto map = imp_->items_;
    auto& newPrimary = map[incomingID];
    newPrimary.reset(new Item(item->SetPrimary(true)));

    OT_ASSERT(newPrimary);

    if (haveExistingPrimary) {
        auto& oldPrimary = map.at(imp_->primary_);

        OT_ASSERT(oldPrimary);

        oldPrimary.reset(new Item(oldPrimary->SetPrimary(false)));

        OT_ASSERT(oldPrimary);
    }

    return Group(imp_->nym_, imp_->section_, imp_->type_, map);
}

auto Group::begin() const -> Group::ItemMap::const_iterator
{
    return imp_->items_.cbegin();
}

auto Group::Best() const -> std::shared_ptr<Item>
{
    if (0 == imp_->items_.size()) { return {}; }

    if (false == imp_->primary_->empty()) {
        return imp_->items_.at(imp_->primary_);
    }

    for (const auto& it : imp_->items_) {
        const auto& claim = it.second;

        OT_ASSERT(claim);

        if (claim->isActive()) { return claim; }
    }

    return imp_->items_.begin()->second;
}

auto Group::Claim(const Identifier& item) const -> std::shared_ptr<Item>
{
    auto it = imp_->items_.find(item);

    if (imp_->items_.end() == it) { return {}; }

    return it->second;
}

auto Group::Delete(const Identifier& id) const -> Group
{
    const bool exists = (1 == imp_->items_.count(id));

    if (false == exists) { return *this; }

    auto map = imp_->items_;
    map.erase(id);

    return Group(imp_->nym_, imp_->section_, imp_->type_, map);
}

auto Group::end() const -> Group::ItemMap::const_iterator
{
    return imp_->items_.cend();
}

auto Group::HaveClaim(const Identifier& item) const -> bool
{
    return (1 == imp_->items_.count(item));
}

auto Group::Primary() const -> const Identifier& { return imp_->primary_; }

auto Group::SerializeTo(proto::ContactSection& section, const bool withIDs)
    const -> bool
{
    if (translate(section.name()) != imp_->section_) {
        LogError()(OT_PRETTY_CLASS())(
            "Trying to serialize to incorrect section.")
            .Flush();

        return false;
    }

    for (const auto& it : imp_->items_) {
        const auto& item = it.second;

        OT_ASSERT(item);

        item->Serialize(*section.add_item(), withIDs);
    }

    return true;
}

auto Group::PrimaryClaim() const -> std::shared_ptr<Item>
{
    if (imp_->primary_->empty()) { return {}; }

    return imp_->items_.at(imp_->primary_);
}

auto Group::Size() const -> std::size_t { return imp_->items_.size(); }

auto Group::Type() const -> const claim::ClaimType& { return imp_->type_; }

Group::~Group() = default;
}  // namespace opentxs::identity::wot::claim
